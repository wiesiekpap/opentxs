// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/Proto.tpp"

#include "core/Shutdown.hpp"
#include "core/StateMachine.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include <mutex>

#include "FilterOracle.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::FilterOracle::"

namespace opentxs
{
auto Factory::BlockchainFilterOracle(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::FilterDatabase& database,
    const std::string& shutdown) -> blockchain::client::internal::FilterOracle*
{
    using ReturnType = blockchain::client::implementation::FilterOracle;

    return new ReturnType(api, network, database, shutdown);
}
}  // namespace opentxs

namespace opentxs::blockchain::client::implementation
{
const std::chrono::seconds FilterOracle::FilterQueue::timeout_{15};
const std::chrono::seconds FilterOracle::RequestQueue::limit_{15};

FilterOracle::FilterOracle(
    const api::internal::Core& api,
    const internal::Network& network,
    const internal::FilterDatabase& database,
    const std::string& shutdown) noexcept
    : internal::FilterOracle()
    , StateMachine(std::bind(&FilterOracle::state_machine, this))
    , api_(api)
    , network_(network)
    , database_(database)
    , running_(Flag::Factory(true))
    , lock_()
    , header_requests_(api_)
    , outstanding_filters_(api_)
    , pipeline_(
          api_.ZeroMQ().Pipeline(api_, [this](auto& in) { pipeline(in); }))
    , shutdown_(
          api.ZeroMQ(),
          {api.Endpoints().Shutdown(), shutdown},
          [this](auto& promise) { this->shutdown(promise); })
{
}

FilterOracle::FilterQueue::FilterQueue(const api::Core& api) noexcept
    : running_(false)
    , queued_(0)
    , filters_()
    , last_received_()
    , target_(make_blank<block::Position>::value(api))
{
    filters_.reserve(1000);
}

FilterOracle::RequestQueue::RequestQueue(const api::Core& api) noexcept
    : lock_()
    , highest_(make_blank<block::Position>::value(api))
    , hashes_()
{
}

auto FilterOracle::FilterQueue::AddFilter(
    const block::Height height,
    const block::Hash& hash,
    std::unique_ptr<const blockchain::internal::GCS> filter) noexcept -> void
{
    OT_ASSERT(filter);

    const auto& target = target_.first;

    if (height > target) {
        LogOutput(OT_METHOD)("FilterQueue::")(__FUNCTION__)(
            ": Filter height (")(height)(") is after requested range")
            .Flush();

        return;
    }

    const auto offset = static_cast<std::size_t>(target - height);

    if (offset >= filters_.size()) {
        LogOutput(OT_METHOD)("FilterQueue::")(__FUNCTION__)(
            ": Filter height (")(height)(") is before requested range")
            .Flush();

        return;
    }

    auto it{filters_.begin()};
    std::advance(it, offset);

    if (hash != it->first) {
        LogOutput(OT_METHOD)("FilterQueue::")(__FUNCTION__)(
            ": Wrong block for filter at height ")(height)
            .Flush();

        return;
    }

    it->second.reset(filter.release());
    ++queued_;
    last_received_ = Clock::now();
}

auto FilterOracle::FilterQueue::Flush(Filters& filters) noexcept
    -> block::Position
{
    for (auto it{filters_.rbegin()}; it != filters_.rend(); ++it) {
        filters.emplace_back(internal::FilterDatabase::Filter{
            it->first->Bytes(), std::move(it->second)});
    }

    running_ = false;
    queued_ = 0;

    return target_;
}

auto FilterOracle::FilterQueue::IsFull() noexcept -> bool
{
    return filters_.size() == queued_;
}

auto FilterOracle::FilterQueue::IsRunning() noexcept -> bool
{
    if (running_) {
        if ((Clock::now() - last_received_) > timeout_) { running_ = false; }
    }

    return running_;
}

auto FilterOracle::FilterQueue::Queue(
    const block::Height startHeight,
    const block::Hash& stopHash,
    const client::HeaderOracle& headers) noexcept -> void
{
    filters_.clear();
    auto header = headers.LoadHeader(stopHash);

    OT_ASSERT(header);

    target_ = header->Position();
    filters_.emplace_back(FilterData{header->Hash(), nullptr});

    while (header->Height() > startHeight) {
        header = headers.LoadHeader(header->ParentHash());

        OT_ASSERT(header);

        filters_.emplace_back(FilterData{header->Hash(), nullptr});
    }

    running_ = true;
    queued_ = 0;
    last_received_ = Clock::now();
}

auto FilterOracle::RequestQueue::Finish(const block::Hash& block) noexcept
    -> void
{
    Lock lock(lock_);
    prune(lock);
    hashes_.erase(block);
}

auto FilterOracle::RequestQueue::IsRunning(const block::Hash& block) const
    noexcept -> bool
{
    Lock lock(lock_);
    prune(lock);

    return 0 < hashes_.count(block);
}

auto FilterOracle::RequestQueue::prune(const Lock& lock) const noexcept -> void
{
    for (auto i = hashes_.begin(); i != hashes_.end();) {
        const auto duration = Clock::now() - i->second;

        if ((duration > limit_) || (std::chrono::seconds(0) > duration)) {
            i = hashes_.erase(i);
        } else {
            ++i;
        }
    }
}

auto FilterOracle::RequestQueue::size() const noexcept -> std::size_t
{
    Lock lock(lock_);
    prune(lock);

    return hashes_.size();
}

auto FilterOracle::RequestQueue::Start(const block::Hash& block) noexcept
    -> void
{
    Lock lock(lock_);
    prune(lock);
    hashes_.emplace(block, Clock::now());
}

auto FilterOracle::AddFilter(
    const filter::Type type,
    const block::Hash& block,
    const Data& filter) const noexcept -> void
{
    if (false == running_.get()) { return; }

    auto work = zmq::Message::Factory();
    work->AddFrame(Work{Work::cfilter});
    work->AddFrame();
    work->AddFrame(Data::Factory(&type, sizeof(type)));
    work->AddFrame(block);
    work->AddFrame(filter);
    pipeline_->Push(work);
}

auto FilterOracle::AddHeaders(
    const filter::Type type,
    const ReadView stopBlock,
    const ReadView previousHeader,
    const std::vector<ReadView> hashes) const noexcept -> void
{
    if (false == running_.get()) { return; }

    auto work = zmq::Message::Factory();
    work->AddFrame(Work{Work::cfheader});
    work->AddFrame(type);
    work->AddFrame(stopBlock.data(), stopBlock.size());
    work->AddFrame(previousHeader.data(), previousHeader.size());
    work->AddFrame();

    for (const auto& hash : hashes) {
        work->AddFrame(hash.data(), hash.size());
    }

    pipeline_->Push(work);
}

auto FilterOracle::CheckBlocks() const noexcept -> void
{
    if (false == running_.get()) { return; }

    Trigger();
}

void FilterOracle::check_filters(
    const filter::Type type,
    const block::Height maxRequests,
    Cleanup& repeat) noexcept
{
    const auto& headers = network_.HeaderOracle();
    const auto [start, best] = headers.CommonParent(database_.FilterTip(type));

    if (start == best) { return; }

    repeat.On();

    if (outstanding_filters_.IsRunning()) { return; }

    const auto headerTip = database_.FilterHeaderTip(type).first;
    const auto begin{start.first + static_cast<block::Height>(1)};
    const auto target{begin + maxRequests - static_cast<block::Height>(1)};
    const auto stopHeight = std::min(std::min(target, headerTip), best.first);

    if (0 > (stopHeight - begin)) { return; }

    const auto stopHash = headers.BestHash(stopHeight);
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Requesting filters from ")(begin)(
        " to ")(stopHeight)
        .Flush();
    outstanding_filters_.Queue(begin, stopHash, headers);
    network_.RequestFilters(type, begin, stopHash);
}

void FilterOracle::check_headers(
    const filter::Type type,
    const block::Height maxRequests,
    Cleanup& repeat) noexcept
{
    const auto& headers = network_.HeaderOracle();
    const auto [start, best] =
        headers.CommonParent(database_.FilterHeaderTip(type));

    if (start == best) {
        repeat.Off();

        return;
    }

    const auto begin{start.first + static_cast<block::Height>(1)};
    const auto target{begin + maxRequests - static_cast<block::Height>(1)};
    const auto stopHeight = std::min(target, best.first);
    const auto stopHash = headers.BestHash(stopHeight);

    if (header_requests_.IsRunning(stopHash)) { return; }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Requesting filter headers from ")(
        begin)(" to ")(stopHeight)
        .Flush();
    header_requests_.Start(stopHash);
    network_.RequestFilterHeaders(type, begin, stopHash);
}

auto FilterOracle::pipeline(const zmq::Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto& header = in.Header();

    if (1 > header.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = header.at(0).as<Work>();

    switch (work) {
        case Work::cfilter: {
            process_cfilter(in);
        } break;
        case Work::cfheader: {
            process_cfheader(in);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto FilterOracle::process_cfheader(const zmq::Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto& params = in.Header();

    if (4 > params.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto type = params.at(1).as<filter::Type>();
    const auto stopBlock = params.at(2).Bytes();
    const auto previousHeader = params.at(3).Bytes();
    const auto& hashes = in.Body();
    const auto stopHash = api_.Factory().Data(stopBlock);
    const auto& headers = network_.HeaderOracle();
    const auto pHeader = headers.LoadHeader(stopHash);

    if (false == bool(pHeader)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Block header for ")(
            stopHash->asHex())(" not found")
            .Flush();

        return;
    }

    const auto& header = *pHeader;

    if (0 > header.Height()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Block ")(stopHash->asHex())(
            " is disconnected")
            .Flush();

        return;
    }

    const auto start =
        header.Height() - (static_cast<block::Height>(hashes.size()) - 1);

    if (0 > start) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Too many hashes provided")
            .Flush();

        return;
    }

    Lock lock(lock_);
    const auto [previousHeight, previousHash] = database_.FilterHeaderTip(type);

    if (header.Height() <= previousHeight) { return; }

    const auto previous =
        database_.LoadFilterHeader(type, previousHash->Bytes());
    auto priorFilter = previous->Bytes();

    if (0 == start) {
        priorFilter = {};
    } else {
        const auto compare = api_.Factory().Data(previousHeader);

        if (compare.get() != previous) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid previous header")
                .Flush();

            return;
        }
    }

    auto output = std::vector<internal::FilterDatabase::Header>{};
    auto n = int{-1};

    for (auto i{start}; i <= header.Height(); ++i) {
        const auto hash = hashes.at(++n).Bytes();
        const auto& row = output.emplace_back(internal::FilterDatabase::Header{
            headers.BestHash(i),
            blockchain::internal::FilterHashToHeader(api_, hash, priorFilter),
            hash});
        priorFilter = std::get<1>(row)->Bytes();
    }

    if (false == headers.IsInBestChain(stopHash)) { return; }

    if (database_.StoreFilterHeaders(type, previousHeader, std::move(output))) {
        if (database_.SetFilterHeaderTip(type, header.Position())) {
            LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
                " filter header chain updated to height ")(header.Height())
                .Flush();
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed updating filter header tip")
                .Flush();
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed saving filter headers")
            .Flush();
    }
}

auto FilterOracle::process_cfilter(const zmq::Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto& body = in.Body();

    if (3 != body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto type = body.at(0).as<filter::Type>();
    const auto block = Data::Factory(body.at(1));
    const auto serialized = proto::Factory<proto::GCS>(body.at(2));
    auto gcs = std::unique_ptr<const blockchain::internal::GCS>{
        Factory::GCS(api_, serialized)};

    if (false == bool(gcs)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid GCS").Flush();

        return;
    }

    const auto pHeader = network_.HeaderOracle().LoadHeader(block);
    const auto& header = *pHeader;

    if (false == bool(pHeader)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load block header ")(
            block->asHex())
            .Flush();

        return;
    }

    const auto hash = gcs->Hash();
    const auto expected = database_.LoadFilterHash(type, block->Bytes());

    if (hash != expected) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Filter for block ")(
            block->asHex())(" at height ")(header.Height())(
            " does not match header. Received: ")(hash->asHex())(" expected: ")(
            expected->asHex())
            .Flush();

        return;
    }

    {
        Lock lock(lock_);
        outstanding_filters_.AddFilter(
            header.Height(), header.Hash(), std::move(gcs));

        if (outstanding_filters_.IsFull()) {
            auto filters = std::vector<internal::FilterDatabase::Filter>{};
            auto position = outstanding_filters_.Flush(filters);

            if (false == database_.StoreFilters(type, std::move(filters))) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Database error").Flush();
                Trigger();

                return;
            }

            database_.SetFilterTip(type, position);
            LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
                " filter chain updated to height ")(header.Height())
                .Flush();
        } else {
            LogVerbose(blockchain::internal::DisplayString(network_.Chain()))(
                " filter for block ")(header.Hash().asHex())(" at height ")(
                header.Height())(" cached")
                .Flush();
        }
    }

    Trigger();
}

auto FilterOracle::Shutdown() noexcept -> std::shared_future<void>
{
    shutdown_.Close();

    if (running_.get()) { shutdown(shutdown_.promise_); }

    return shutdown_.future_;
}

auto FilterOracle::shutdown(std::promise<void>& promise) noexcept -> void
{
    running_->Off();
    Stop().get();
    pipeline_->Close();

    try {
        promise.set_value();
    } catch (...) {
    }
}

auto FilterOracle::Start() noexcept -> void
{
    if (false == running_.get()) { return; }

    Trigger();
}

auto FilterOracle::state_machine() noexcept -> bool
{
    static const auto type = filter::Type::Basic;

    auto repeat = Cleanup{lock_};
    check_headers(type, 2000, repeat);
    check_filters(type, 1000, repeat);

    return repeat;
}

FilterOracle::~FilterOracle()
{
    Shutdown().get();
    shutdown_.Close();
}
}  // namespace opentxs::blockchain::client::implementation

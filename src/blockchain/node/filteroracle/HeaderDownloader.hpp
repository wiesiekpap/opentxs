// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/filteroracle/FilterOracle.hpp"  // IWYU pragma: associated

#include <functional>

#include "blockchain/DownloadManager.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::implementation
{
using HeaderDM = download::Manager<
    FilterOracle::HeaderDownloader,
    cfilter::Hash,
    cfilter::Header,
    cfilter::Type>;
using HeaderWorker = Worker<api::Session>;

class FilterOracle::HeaderDownloader final : public HeaderDM,
                                             public HeaderWorker
{
public:
    using Callback =
        std::function<Position(const Position&, const cfilter::Header&)>;

    auto NextBatch() noexcept { return allocate_batch(type_); }

    HeaderDownloader(
        const api::Session& api,
        internal::FilterDatabase& db,
        const HeaderOracle& header,
        const internal::Network& node,
        FilterOracle::FilterDownloader& filter,
        const blockchain::Type chain,
        const cfilter::Type type,
        const UnallocatedCString& shutdown,
        Callback&& cb) noexcept
        : HeaderDM(
              [&] { return db.FilterHeaderTip(type); }(),
              [&] {
                  auto promise = std::promise<cfilter::Header>{};
                  const auto tip = db.FilterHeaderTip(type);
                  promise.set_value(
                      db.LoadFilterHeader(type, tip.second.Bytes()));

                  return Finished{promise.get_future()};
              }(),
              "cfheader",
              20000,
              10000)
        , HeaderWorker(api, 20ms)
        , db_(db)
        , header_(header)
        , node_(node)
        , filter_(filter)
        , chain_(chain)
        , type_(type)
        , checkpoint_(std::move(cb))
    {
        init_executor(
            {shutdown, UnallocatedCString{api_.Endpoints().BlockchainReorg()}});

        OT_ASSERT(checkpoint_);
    }

    ~HeaderDownloader() final
    {
        try {
            signal_shutdown().get();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            // TODO MT-34 improve
        }
    }

protected:
    auto pipeline(zmq::Message&& in) noexcept -> void final;
    auto state_machine() noexcept -> bool final;

private:
    auto shut_down() noexcept -> void;

private:
    friend HeaderDM;

    internal::FilterDatabase& db_;
    const HeaderOracle& header_;
    const internal::Network& node_;
    FilterOracle::FilterDownloader& filter_;
    const blockchain::Type chain_;
    const cfilter::Type type_;
    const Callback checkpoint_;

    auto batch_ready() const noexcept -> void
    {
        node_.JobReady(internal::PeerManager::Task::JobAvailableCfheaders);
    }
    auto batch_size(const std::size_t in) const noexcept -> std::size_t
    {
        if (in < 10) {

            return 1;
        } else if (in < 100) {

            return 10;
        } else if (in < 1000) {

            return 100;
        } else {

            return 2000;
        }
    }
    auto check_task(TaskType&) const noexcept -> void {}
    auto trigger_state_machine() const noexcept -> void { trigger(); }
    auto update_tip(const Position& position, const cfilter::Header&)
        const noexcept -> void
    {
        const auto saved = db_.SetFilterHeaderTip(type_, position);

        OT_ASSERT(saved);

        LogDetail()(print(chain_))(" cfheader chain updated to height ")(
            position.first)
            .Flush();
        filter_.UpdatePosition(position);
    }

    auto process_position(const zmq::Message& in) noexcept -> void
    {
        {
            const auto body = in.Body();

            OT_ASSERT(body.size() >= 4);

            const auto chain = body.at(1).as<blockchain::Type>();

            if (chain_ != chain) { return; }
        }

        process_position();
    }
    auto process_position() noexcept -> void
    {
        auto current = known();
        auto hashes = header_.BestChain(current, 20000);

        OT_ASSERT(0 < hashes.size());

        auto prior = Previous{std::nullopt};
        {
            auto& first = hashes.front();

            if (first != current) {
                auto promise = std::promise<cfilter::Header>{};
                promise.set_value(
                    db_.LoadFilterHeader(type_, first.second.Bytes()));
                prior.emplace(std::move(first), promise.get_future());
            }
        }
        hashes.erase(hashes.begin());
        update_position(std::move(hashes), type_, std::move(prior));
    }
    auto process_reset(const zmq::Message& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(3 < body.size());

        auto position =
            Position{body.at(1).as<block::Height>(), body.at(2).Bytes()};
        auto promise = std::promise<cfilter::Header>{};
        promise.set_value(body.at(3).Bytes());
        Reset(position, promise.get_future());
    }
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        if (0 == data.size()) { return; }

        const auto& previous = data.front()->previous_.get();
        auto hashes = Vector<cfilter::Hash>{};
        auto headers = Vector<internal::FilterDatabase::CFHeaderParams>{};

        for (const auto& task : data) {
            const auto& hash = hashes.emplace_back(task->data_.get());
            auto header = blockchain::internal::FilterHashToHeader(
                api_, hash.Bytes(), task->previous_.get().Bytes());
            const auto& position = task->position_;
            const auto check = checkpoint_(position, header);

            if (check == position) {
                headers.emplace_back(position.second, header, hash);
                task->process(std::move(header));
            } else {
                const auto good =
                    db_.LoadFilterHeader(type_, check.second.Bytes());

                OT_ASSERT(false == good.IsNull());

                auto work = MakeWork(Work::reset_filter_tip);
                work.AddFrame(check.first);
                work.AddFrame(check.second);
                work.AddFrame(good);
                pipeline_.Push(std::move(work));
            }
        }

        const auto saved =
            db_.StoreFilterHeaders(type_, previous.Bytes(), std::move(headers));

        OT_ASSERT(saved);
    }
};

auto FilterOracle::HeaderDownloader::pipeline(zmq::Message&& in) noexcept
    -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(1 <= body.size());

    const auto work = [&] {
        try {

            return body.at(0).as<FilterOracle::Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case FilterOracle::Work::shutdown: {
            protect_shutdown([this] { shut_down(); });
        } break;
        case FilterOracle::Work::block:
        case FilterOracle::Work::reorg: {
            process_position(in);
            run_if_enabled();
        } break;
        case FilterOracle::Work::reset_filter_tip: {
            process_reset(in);
        } break;
        case FilterOracle::Work::heartbeat: {
            process_position();
            run_if_enabled();
        } break;
        case FilterOracle::Work::statemachine: {
            run_if_enabled();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto FilterOracle::HeaderDownloader::state_machine() noexcept -> bool
{
    return HeaderDM::state_machine();
}

auto FilterOracle::HeaderDownloader::shut_down() noexcept -> void
{
    close_pipeline();
    // TODO MT-34 investigate what other actions might be needed
}

}  // namespace opentxs::blockchain::node::implementation

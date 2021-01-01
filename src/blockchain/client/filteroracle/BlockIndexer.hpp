// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include <functional>

#include "blockchain/DownloadManager.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs::blockchain::client::implementation
{
using BlockDM = download::Manager<
    FilterOracle::BlockIndexer,
    std::shared_ptr<const block::bitcoin::Block>,
    filter::pHeader,
    filter::Type>;
using BlockWorker = Worker<FilterOracle::BlockIndexer, api::Core>;

class FilterOracle::BlockIndexer : public BlockDM, public BlockWorker
{
public:
    using Callback = std::function<std::unique_ptr<const GCS>(
        const filter::Type type,
        const block::bitcoin::Block& block)>;

    auto Heartbeat() noexcept -> void
    {
        process_position(block_.Tip());
        trigger();
    }
    auto NextBatch() noexcept { return allocate_batch(type_); }
    auto Start() noexcept { init_promise_.set_value(); }

    BlockIndexer(
        const api::Core& api,
        const internal::FilterDatabase& db,
        const internal::HeaderOracle& header,
        const internal::BlockOracle& block,
        const internal::Network& network,
        const blockchain::Type chain,
        const filter::Type type,
        const std::string& shutdown,
        Callback cb) noexcept
        : BlockDM(
              [&] { return db.FilterTip(type); }(),
              [&] {
                  auto promise = std::promise<filter::pHeader>{};
                  const auto tip = db.FilterTip(type);
                  promise.set_value(
                      db.LoadFilterHeader(type, tip.second->Bytes()));

                  return Finished{promise.get_future()};
              }(),
              "filter")
        , BlockWorker(api, std::chrono::milliseconds{20})
        , db_(db)
        , header_(header)
        , block_(block)
        , network_(network)
        , chain_(chain)
        , type_(type)
        , cb_(std::move(cb))
        , socket_(api_.ZeroMQ().PublishSocket())
        , init_promise_()
        , init_(init_promise_.get_future())
    {
        init_executor(
            {shutdown,
             api_.Endpoints().InternalBlockchainBlockUpdated(chain_)});
        auto zmq = socket_->Start(
            api_.Endpoints().InternalBlockchainFilterUpdated(chain_));

        OT_ASSERT(zmq);
        OT_ASSERT(cb_);
    }

    ~BlockIndexer() { stop_worker().get(); }

private:
    friend BlockDM;
    friend BlockWorker;

    const internal::FilterDatabase& db_;
    const internal::HeaderOracle& header_;
    const internal::BlockOracle& block_;
    const internal::Network& network_;
    const blockchain::Type chain_;
    const filter::Type type_;
    const Callback cb_;
    OTZMQPublishSocket socket_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    auto batch_ready() const noexcept -> void { trigger(); }
    auto batch_size(const std::size_t in) const noexcept -> std::size_t
    {
        if (in < 10) {

            return 1;
        } else if (in < 100) {

            return 10;
        } else if (in < 1000) {

            return 100;
        } else {

            return 1000;
        }
    }
    auto check_task(TaskType&) const noexcept -> void {}
    auto trigger_state_machine() const noexcept -> void { trigger(); }
    auto update_tip(const Position& position, const filter::pHeader&)
        const noexcept -> void
    {
        auto saved = db_.SetFilterTip(type_, position);

        OT_ASSERT(saved);

        saved = db_.SetFilterHeaderTip(type_, position);

        OT_ASSERT(saved);

        LogVerbose(DisplayString(chain_))(
            " cfheader and cfilter chain updated to height ")(position.first)
            .Flush();
        auto work = MakeWork(OT_ZMQ_NEW_FILTER_SIGNAL);
        work->AddFrame(type_);
        work->AddFrame(position.first);
        work->AddFrame(position.second);
        socket_->Send(work);
    }

    auto download() noexcept -> void
    {
        auto work = NextBatch();
        constexpr auto none = std::chrono::seconds{0};

        for (const auto& task : work.data_) {
            const auto& hash = task->position_.second;
            auto future = block_.LoadBitcoin(hash);

            if (std::future_status::ready == future.wait_for(none)) {
                auto block = future.get();

                OT_ASSERT(block);
                OT_ASSERT(block->ID() == hash);

                task->download(std::move(block));
            }
        }
    }
    auto pipeline(const zmq::Message& in) noexcept -> void
    {
        init_.get();

        if (false == running_.get()) { return; }

        const auto body = in.Body();

        OT_ASSERT(1 <= body.size());

        using Work = FilterOracle::Work;
        const auto work = body.at(0).as<Work>();

        switch (work) {
            case Work::full_block: {
                process_position(in);
                do_work();
            } break;
            case Work::statemachine: {
                download();
                do_work();
            } break;
            case Work::shutdown: {
                shutdown(shutdown_promise_);
            } break;
            default: {
                OT_FAIL;
            }
        }
    }
    auto process_position(const zmq::Message& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(body.size() > 2);

        process_position(Position{
            body.at(1).as<block::Height>(), api_.Factory().Data(body.at(2))});
    }
    auto process_position(const Position& pos) noexcept -> void
    {
        try {
            auto current = known();
            auto hashes = header_.Ancestors(current, pos);

            OT_ASSERT(0 < hashes.size());

            auto prior = Previous{std::nullopt};
            {
                auto& first = hashes.front();

                if (first != current) {
                    auto promise = std::promise<filter::pHeader>{};
                    auto header =
                        db_.LoadFilterHeader(type_, first.second->Bytes());

                    OT_ASSERT(false == header->empty());

                    promise.set_value(std::move(header));
                    prior.emplace(std::move(first), promise.get_future());
                }
            }
            hashes.erase(hashes.begin());
            update_position(std::move(hashes), type_, std::move(prior));
        } catch (...) {
        }
    }
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        if (0 == data.size()) { return; }

        const auto& tip = data.back();
        auto blockHashes = std::vector<block::pHash>{};
        auto filterHashes = std::vector<filter::pHash>{};
        auto filters = std::vector<internal::FilterDatabase::Filter>{};
        auto headers = std::vector<internal::FilterDatabase::Header>{};
        const auto count = data.size();
        blockHashes.reserve(count);
        filterHashes.reserve(count);
        filters.reserve(count);
        headers.reserve(count);

        for (const auto& task : data) {
            try {
                const auto& blockhash = task->position_.second.get();
                const auto& pBlock = task->data_.get();

                if (false == bool(pBlock)) {
                    throw std::runtime_error(
                        std::string{"failed to load block "} +
                        blockhash.asHex());
                }

                const auto& block = *pBlock;
                const auto& id = blockHashes.emplace_back(blockhash).get();
                const auto& pGCS =
                    filters.emplace_back(id.Bytes(), cb_(type_, block)).second;

                if (false == bool(pGCS)) {
                    throw std::runtime_error(
                        std::string{"failed to calculate gcs for "} +
                        id.asHex());
                }

                const auto& gcs = *pGCS;
                const auto& previousHeader = task->previous_.get().get();
                const auto prior = previousHeader.Bytes();
                const auto& filterHash = filterHashes.emplace_back(gcs.Hash());
                const auto& it = headers.emplace_back(
                    id, gcs.Header(prior), filterHash->Bytes());
                const auto& [bHash, header, fHash] = it;

                if (header->empty()) {
                    throw std::runtime_error(
                        std::string{"failed to calculate filter header for "} +
                        id.asHex());
                }

                task->process(filter::pHeader{header});
            } catch (...) {
                task->redownload();
                break;
            }
        }

        const auto stored =
            db_.StoreFilters(type_, headers, filters, tip->position_);

        if (false == stored) { OT_FAIL; }
    }
    auto shutdown(std::promise<void>& promise) noexcept -> void
    {
        init_.get();

        if (running_->Off()) {
            try {
                promise.set_value();
            } catch (...) {
            }
        }
    }
};
}  // namespace opentxs::blockchain::client::implementation

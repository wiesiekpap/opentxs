// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/client/BlockOracle.hpp"  // IWYU pragma: associated

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
    BlockOracle::BlockDownloader,
    std::shared_ptr<const block::bitcoin::Block>,
    int>;
using BlockWorker = Worker<BlockOracle::BlockDownloader, api::Core>;

class BlockOracle::BlockDownloader : public BlockDM, public BlockWorker
{
public:
    auto Heartbeat() noexcept -> void
    {
        process_position();
        trigger();
    }
    auto NextBatch() noexcept { return allocate_batch(0); }
    auto Start() noexcept { init_promise_.set_value(); }

    BlockDownloader(
        const api::Core& api,
        const internal::BlockDatabase& db,
        const internal::HeaderOracle& header,
        const internal::Network& network,
        const blockchain::Type chain,
        const std::string& shutdown) noexcept
        : BlockDM(
              [&] { return db.BlockTip(); }(),
              [&] {
                  auto promise = std::promise<int>{};
                  promise.set_value(0);

                  return Finished{promise.get_future()};
              }(),
              "block")
        , BlockWorker(api, std::chrono::milliseconds{20})
        , db_(db)
        , header_(header)
        , network_(network)
        , chain_(chain)
        , socket_(api_.ZeroMQ().PublishSocket())
        , init_promise_()
        , init_(init_promise_.get_future())
    {
        init_executor({shutdown, api_.Endpoints().BlockchainReorg()});
        auto zmq = socket_->Start(
            api_.Endpoints().InternalBlockchainBlockUpdated(chain_));

        OT_ASSERT(zmq);
    }

    ~BlockDownloader() { stop_worker().get(); }

private:
    friend BlockDM;
    friend BlockWorker;

    const internal::BlockDatabase& db_;
    const internal::HeaderOracle& header_;
    const internal::Network& network_;
    const blockchain::Type chain_;
    OTZMQPublishSocket socket_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    auto batch_ready() const noexcept -> void
    {
        network_.JobReady(internal::PeerManager::Task::JobAvailableBlock);
    }
    auto batch_size(const std::size_t in) const noexcept -> std::size_t
    {
        if (in < 10) {

            return 1;
        } else if (in < 100) {

            return 10;
        } else {

            return 100;
        }
    }
    auto check_task(TaskType& task) const noexcept -> void
    {
        const auto& hash = task.position_.second;

        if (auto block = db_.BlockLoadBitcoin(hash); bool(block)) {
            task.download(std::move(block));
        }
    }
    auto trigger_state_machine() const noexcept -> void { trigger(); }
    auto update_tip(const Position& position, const int&) const noexcept -> void
    {
        const auto saved = db_.SetBlockTip(position);

        OT_ASSERT(saved);

        LogVerbose(DisplayString(chain_))(" block chain updated to height ")(
            position.first)
            .Flush();
        auto work = MakeWork(OT_ZMQ_NEW_FULL_BLOCK_SIGNAL);
        work->AddFrame(position.first);
        work->AddFrame(position.second);
        socket_->Send(work);
    }

    auto pipeline(const zmq::Message& in) noexcept -> void
    {
        init_.get();

        if (false == running_.get()) { return; }

        const auto body = in.Body();

        OT_ASSERT(1 <= body.size());

        const auto work = body.at(0).as<BlockOracle::Work>();

        switch (work) {
            case BlockOracle::Work::block:
            case BlockOracle::Work::reorg: {
                process_position(in);
                do_work();
            } break;
            case BlockOracle::Work::statemachine: {
                do_work();
            } break;
            case BlockOracle::Work::shutdown: {
                shutdown(shutdown_promise_);
            } break;
            default: {
                OT_FAIL;
            }
        }
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
        auto hashes = header_.BestChain(current);

        OT_ASSERT(0 < hashes.size());

        auto prior = Previous{std::nullopt};
        {
            auto& first = hashes.front();

            if (first != current) {
                auto promise = std::promise<int>{};
                promise.set_value(0);
                prior.emplace(std::move(current), promise.get_future());
            }
        }
        hashes.erase(hashes.begin());
        update_position(std::move(hashes), 0, std::move(prior));
    }
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        if (0 == data.size()) { return; }

        for (const auto& task : data) {
            const auto& pBlock = task->data_.get();

            OT_ASSERT(pBlock);

            const auto& block = *pBlock;

            // TODO implement batch db updates
            if (db_.BlockStore(block)) {
                task->process(0);
            } else {
                task->redownload();
                break;
            }
        }
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

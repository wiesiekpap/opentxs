// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/client/BlockOracle.hpp"  // IWYU pragma: associated

#include <memory>
#include <vector>

#include "blockchain/client/blockoracle/BlockDownloader.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/client/Factory.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/blockchain/client/BlockOracle.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::BlockOracle::"

namespace opentxs::factory
{
auto BlockOracle(
    const api::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::HeaderOracle& header,
    const blockchain::client::internal::BlockDatabase& db,
    const blockchain::Type chain,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::BlockOracle>
{
    using ReturnType = blockchain::client::implementation::BlockOracle;

    return std::make_unique<ReturnType>(
        api, network, header, db, chain, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::client::implementation
{
BlockOracle::BlockOracle(
    const api::Core& api,
    const internal::Network& network,
    const internal::HeaderOracle& header,
    const internal::BlockDatabase& db,
    const blockchain::Type chain,
    const std::string& shutdown) noexcept
    : Worker(api, std::chrono::milliseconds{25})
    , network_(network)
    , db_(db)
    , init_promise_()
    , init_(init_promise_.get_future())
    , cache_(api, network, db, chain)
    , lock_()
    , block_downloader_([&]() -> std::unique_ptr<BlockDownloader> {
        using Policy = api::client::blockchain::BlockStorage;

        if (Policy::All != db.BlockPolicy()) { return nullptr; }

        return std::make_unique<BlockDownloader>(
            api_, db, header, network, chain, shutdown);
    }())
{
    init_executor({shutdown});
}

auto BlockOracle::GetBlockJob() const noexcept -> BlockJob
{
    auto lock = Lock{lock_};

    if (block_downloader_) {

        return block_downloader_->NextBatch();
    } else {

        return {};
    }
}

auto BlockOracle::Heartbeat() const noexcept -> void
{
    trigger();
    auto lock = Lock{lock_};

    if (block_downloader_) { block_downloader_->Heartbeat(); }
}

auto BlockOracle::Init() noexcept -> void
{
    init_promise_.set_value();
    auto lock = Lock{lock_};

    if (block_downloader_) { block_downloader_->Start(); }
}

auto BlockOracle::LoadBitcoin(const block::Hash& block) const noexcept
    -> BitcoinBlockFuture
{
    auto output = cache_.Request(block);
    trigger();

    return output;
}

auto BlockOracle::LoadBitcoin(const BlockHashes& hashes) const noexcept
    -> BitcoinBlockFutures
{
    auto output = cache_.Request(hashes);
    trigger();

    OT_ASSERT(hashes.size() == output.size());

    return output;
}

auto BlockOracle::pipeline(const zmq::Message& in) noexcept -> void
{
    init_.get();

    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    switch (body.at(0).as<Task>()) {
        case Task::ProcessBlock: {
            if (2 > body.size()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": No block").Flush();

                OT_FAIL;
            }

            cache_.ReceiveBlock(body.at(1));
            [[fallthrough]];
        }
        case Task::StateMachine: {
            do_work();
        } break;
        case Task::Shutdown: {
            shutdown(shutdown_promise_);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto BlockOracle::shutdown(std::promise<void>& promise) noexcept -> void
{
    init_.get();
    {
        auto lock = Lock{lock_};

        if (block_downloader_) { block_downloader_.reset(); }
    }

    if (running_->Off()) {
        cache_.Shutdown();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto BlockOracle::state_machine() noexcept -> bool
{
    LogTrace(OT_METHOD)(__FUNCTION__).Flush();

    if (false == running_.get()) { return false; }

    return cache_.StateMachine();
}

auto BlockOracle::SubmitBlock(const ReadView in) const noexcept -> void
{
    auto work = MakeWork(Task::ProcessBlock);
    work->AddFrame(in.data(), in.size());
    pipeline_->Push(work);
}

BlockOracle::~BlockOracle() { Shutdown().get(); }
}  // namespace opentxs::blockchain::client::implementation

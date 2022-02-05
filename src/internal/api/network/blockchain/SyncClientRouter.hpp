// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <string_view>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace network
{
class Blockchain;
}  // namespace network

class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Thread;
}  // namespace internal
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::api::network::blockchain
{
class SyncClientRouter
{
public:
    enum class Task : OTZMQWorkType {
        Shutdown = value(WorkType::Shutdown),
        BlockHeader = value(WorkType::BlockchainNewHeader),
        Reorg = value(WorkType::BlockchainReorg),
        Server = value(WorkType::SyncServerUpdated),
        Ack = value(WorkType::P2PBlockchainSyncAck),
        Reply = value(WorkType::P2PBlockchainSyncReply),
        Push = value(WorkType::P2PBlockchainNewBlock),
        Register = OT_ZMQ_INTERNAL_SIGNAL + 0,
        Request = OT_ZMQ_INTERNAL_SIGNAL + 1,
        Processed = OT_ZMQ_INTERNAL_SIGNAL + 2,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    auto Endpoint() const noexcept -> std::string_view;

    auto Init(const Blockchain& parent) noexcept -> void;

    SyncClientRouter(const api::Session& api) noexcept;

    ~SyncClientRouter();

private:
    class Imp;

    Imp* imp_;

    SyncClientRouter(
        const api::Session& api,
        opentxs::network::zeromq::internal::Batch& batch) noexcept;
    SyncClientRouter() = delete;
    SyncClientRouter(const SyncClientRouter&) = delete;
    SyncClientRouter(SyncClientRouter&&) = delete;
    auto operator=(const SyncClientRouter&) -> SyncClientRouter& = delete;
    auto operator=(SyncClientRouter&&) -> SyncClientRouter& = delete;
};
}  // namespace opentxs::api::network::blockchain

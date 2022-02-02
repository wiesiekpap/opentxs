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
class SyncClient
{
public:
    enum class Task : OTZMQWorkType {
        Shutdown = value(WorkType::Shutdown),
        Ack = value(WorkType::P2PBlockchainSyncAck),
        Reply = value(WorkType::P2PBlockchainSyncReply),
        Push = value(WorkType::P2PBlockchainNewBlock),
        Server = value(WorkType::SyncServerUpdated),
        Register = OT_ZMQ_INTERNAL_SIGNAL + 0,
        Request = OT_ZMQ_INTERNAL_SIGNAL + 1,
        Processed = OT_ZMQ_INTERNAL_SIGNAL + 2,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    auto Endpoint() const noexcept -> std::string_view;

    auto Init(const Blockchain& parent) noexcept -> void;

    SyncClient(const api::Session& api) noexcept;

    ~SyncClient();

private:
    struct Imp;

    Imp* imp_;

    SyncClient(
        const api::Session& api,
        opentxs::network::zeromq::internal::Batch& batch) noexcept;
    SyncClient() = delete;
    SyncClient(const SyncClient&) = delete;
    SyncClient(SyncClient&&) = delete;
    auto operator=(const SyncClient&) -> SyncClient& = delete;
    auto operator=(SyncClient&&) -> SyncClient& = delete;
};
}  // namespace opentxs::api::network::blockchain

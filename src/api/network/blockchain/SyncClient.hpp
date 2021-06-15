// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "opentxs/blockchain/Blockchain.hpp"
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

class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs::api::network::blockchain
{
struct SyncClient {
    enum class Task : OTZMQWorkType {
        Shutdown = value(WorkType::Shutdown),
        Ack = value(WorkType::SyncAcknowledgement),
        Reply = value(WorkType::SyncReply),
        Push = value(WorkType::NewBlock),
        Server = value(WorkType::SyncServerUpdated),
        Register = OT_ZMQ_INTERNAL_SIGNAL + 0,
        Request = OT_ZMQ_INTERNAL_SIGNAL + 1,
        Processed = OT_ZMQ_INTERNAL_SIGNAL + 2,
    };

    auto Endpoint() const noexcept -> const std::string&;

    auto Init(const Blockchain& parent) noexcept -> void;

    SyncClient(const api::Core& api) noexcept;

    ~SyncClient();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_p_;
    Imp& imp_;

    SyncClient() = delete;
    SyncClient(const SyncClient&) = delete;
    SyncClient(SyncClient&&) = delete;
    auto operator=(const SyncClient&) -> SyncClient& = delete;
    auto operator=(SyncClient&&) -> SyncClient& = delete;
};
}  // namespace opentxs::api::network::blockchain

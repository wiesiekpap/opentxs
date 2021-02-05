// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>
#include <string>

#include "api/client/Blockchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace implementation
{
class Blockchain;
}  // namespace implementation
}  // namespace client

class Core;
}  // namespace api

namespace proto
{
class BlockchainP2PHello;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::api::client::blockchain
{
struct SyncClient {
    using Blockchain = implementation::Blockchain;
    using Chain = opentxs::blockchain::Type;
    using Position = opentxs::blockchain::block::Position;
    using States = std::map<Chain, Position>;

    auto Heartbeat(const proto::BlockchainP2PHello& data) const noexcept
        -> void;
    auto IsActive(const Chain chain) const noexcept -> bool;
    auto IsConnected() const noexcept -> bool;

    SyncClient(
        const api::Core& api,
        Blockchain& parent,
        const std::string& endpoint) noexcept;

    ~SyncClient();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_p_;
    Imp& imp_;
};
}  // namespace opentxs::api::client::blockchain

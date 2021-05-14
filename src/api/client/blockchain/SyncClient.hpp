// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "internal/api/client/Client.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs::api::client::blockchain
{
struct SyncClient {
    using Blockchain = client::internal::Blockchain;
    using Chain = opentxs::blockchain::Type;
    using Position = opentxs::blockchain::block::Position;
    using States = std::map<Chain, Position>;
    using SyncState = std::vector<opentxs::network::blockchain::sync::State>;

    auto Heartbeat(SyncState data) const noexcept -> void;
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

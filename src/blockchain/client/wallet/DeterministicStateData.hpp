// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

#include "blockchain/client/wallet/SubchainStateData.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/BalanceNode.hpp"
#include "opentxs/api/client/blockchain/HD.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/client/BlockOracle.hpp"
#include "opentxs/blockchain/client/FilterOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class Deterministic;
}  // namespace blockchain

namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
}  // namespace bitcoin
}  // namespace block
namespace client
{
namespace internal
{
struct Network;
}  // namespace internal
}  // namespace client
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Push;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Outstanding;
}  // namespace opentxs

namespace opentxs::blockchain::client::wallet
{
class DeterministicStateData final : public SubchainStateData
{
public:
    const api::client::blockchain::Deterministic& node_;

    auto index() noexcept -> void final;

    DeterministicStateData(
        const api::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const internal::Network& network,
        const WalletDatabase& db,
        const api::client::blockchain::Deterministic& node,
        const SimpleCallback& taskFinished,
        Outstanding& jobCounter,
        const zmq::socket::Push& threadPool,
        const filter::Type filter,
        const Subchain subchain) noexcept;

    ~DeterministicStateData() final = default;

private:
    auto type() const noexcept -> std::stringstream final;

    auto check_index() noexcept -> bool final;
    auto handle_confirmed_matches(
        const block::bitcoin::Block& block,
        const block::Position& position,
        const block::Block::Matches& confirmed) noexcept -> void final;

    DeterministicStateData() = delete;
    DeterministicStateData(const DeterministicStateData&) = delete;
    DeterministicStateData(DeterministicStateData&&) = delete;
    DeterministicStateData& operator=(const DeterministicStateData&) = delete;
    DeterministicStateData& operator=(DeterministicStateData&&) = delete;
};
}  // namespace opentxs::blockchain::client::wallet

// Copyright (c) 2010-2020 The Open-Transactions developers
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
class HD;
}  // namespace blockchain

class Blockchain;
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
}  // namespace blockchain

class Outstanding;
}  // namespace opentxs

namespace opentxs::blockchain::client::implementation
{
struct HDStateData {
    using OutstandingMap =
        std::map<block::pHash, BlockOracle::BitcoinBlockFuture>;
    using Subchain = internal::WalletDatabase::Subchain;
    using WalletDatabase = internal::WalletDatabase;
    using ProcessQueue = std::queue<OutstandingMap::iterator>;

    struct ReorgQueue {
        auto Empty() const noexcept -> bool;

        auto Queue(const block::Position& parent) noexcept -> bool;
        auto Next() noexcept -> block::Position;

    private:
        mutable std::mutex lock_{};
        std::queue<block::Position> parents_{};
    };

    const api::Core& api_;
    const api::client::Blockchain& blockchain_;
    const internal::Network& network_;
    const internal::WalletDatabase& db_;
    const api::client::blockchain::HD& node_;
    const SimpleCallback& task_finished_;
    Outstanding& job_counter_;
    const filter::Type filter_type_;
    const Subchain subchain_;
    std::atomic<bool> running_;
    ReorgQueue reorg_;
    std::optional<Bip32Index> last_indexed_;
    std::optional<block::Position> last_scanned_;
    std::vector<block::pHash> blocks_to_request_;
    OutstandingMap outstanding_blocks_;
    ProcessQueue process_block_queue_;

    auto index() noexcept -> void;
    auto process() noexcept -> void;
    auto reorg() noexcept -> void;
    auto scan() noexcept -> void;

    HDStateData(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const internal::Network& network,
        const WalletDatabase& db,
        const api::client::blockchain::HD& node,
        const SimpleCallback& taskFinished,
        Outstanding& jobCounter,
        const filter::Type filter,
        const Subchain subchain) noexcept;

private:
    auto get_targets(
        const internal::WalletDatabase::Patterns& keys,
        const std::vector<internal::WalletDatabase::UTXO>& unspent)
        const noexcept -> client::GCS::Targets;
    auto index_element(
        const filter::Type type,
        const api::client::blockchain::BalanceNode::Element& input,
        const Bip32Index index,
        WalletDatabase::ElementMap& output) noexcept -> void;
    auto update_utxos(
        const block::bitcoin::Block& block,
        const block::Position& position,
        const block::Block::Matches matches) noexcept -> void;

    HDStateData() = delete;
    HDStateData(const HDStateData&) = delete;
    HDStateData(HDStateData&&) = delete;
    HDStateData& operator=(const HDStateData&) = delete;
    HDStateData& operator=(HDStateData&&) = delete;
};
}  // namespace opentxs::blockchain::client::implementation

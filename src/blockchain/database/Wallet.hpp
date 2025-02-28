// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "blockchain/database/wallet/Output.hpp"
#include "blockchain/database/wallet/Proposal.hpp"
#include "blockchain/database/wallet/Subchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BlockchainTransactionProposal.pb.h"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Output;
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Position;
}  // namespace block

namespace database
{
namespace common
{
class Database;
}  // namespace common
}  // namespace database

namespace node
{
namespace internal
{
struct SpendPolicy;
}  // namespace internal

class HeaderOracle;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage

class Data;
class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::implemenation
{
class Wallet
{
public:
    using Parent = database::Wallet;
    using NodeID = Parent::NodeID;
    using pNodeID = Parent::pNodeID;
    using SubchainIndex = Parent::SubchainIndex;
    using pSubchainIndex = Parent::pSubchainIndex;
    using ElementID = Parent::ElementID;
    using ElementMap = Parent::ElementMap;
    using Pattern = Parent::Pattern;
    using Patterns = Parent::Patterns;
    using MatchingIndices = Parent::MatchingIndices;
    using BatchedMatches = Parent::BatchedMatches;
    using UTXO = Parent::UTXO;
    using TXOs = Parent::TXOs;

    auto AddConfirmedTransactions(
        const NodeID& account,
        const SubchainIndex& index,
        BatchedMatches&& transactions,
        TXOs& txoCreated,
        TXOs& txoConsumed) noexcept -> bool;
    auto AddMempoolTransaction(
        const NodeID& balanceNode,
        const crypto::Subchain subchain,
        const Vector<std::uint32_t>& outputIndices,
        const bitcoin::block::Transaction& transaction,
        TXOs& txoCreated) const noexcept -> bool;
    auto AddOutgoingTransaction(
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const bitcoin::block::Transaction& transaction) const noexcept -> bool;
    auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) const noexcept -> bool;
    auto AdvanceTo(const block::Position& pos) const noexcept -> bool;
    auto CancelProposal(const Identifier& id) const noexcept -> bool;
    auto CompletedProposals() const noexcept -> UnallocatedSet<OTIdentifier>;
    auto FinalizeReorg(
        storage::lmdb::LMDB::Transaction& tx,
        const block::Position& pos) const noexcept -> bool;
    auto ForgetProposals(const UnallocatedSet<OTIdentifier>& ids) const noexcept
        -> bool;
    auto GetBalance() const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner) const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner, const NodeID& node)
        const noexcept -> Balance;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance;
    auto GetOutputs(node::TxoState type, alloc::Resource* alloc) const noexcept
        -> Vector<UTXO>;
    auto GetOutputs(
        const identifier::Nym& owner,
        node::TxoState type,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>;
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        node::TxoState type,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>;
    auto GetOutputs(
        const crypto::Key& key,
        node::TxoState type,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>;
    auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<node::TxoTag>;
    auto GetPatterns(const SubchainIndex& index, alloc::Resource* alloc)
        const noexcept -> Patterns;
    auto GetPosition() const noexcept -> block::Position;
    auto GetSubchainID(
        const NodeID& balanceNode,
        const crypto::Subchain subchain) const noexcept -> pSubchainIndex;
    auto GetTransactions() const noexcept -> UnallocatedVector<block::pTxid>;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::pTxid>;
    auto GetUnconfirmedTransactions() const noexcept
        -> UnallocatedSet<block::pTxid>;
    auto GetUnspentOutputs(alloc::Resource* alloc) const noexcept
        -> Vector<UTXO>;
    auto GetUnspentOutputs(
        const NodeID& balanceNode,
        const crypto::Subchain subchain,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>;
    auto GetWalletHeight() const noexcept -> block::Height;
    auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>;
    auto LoadProposals() const noexcept
        -> UnallocatedVector<proto::BlockchainTransactionProposal>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<OTIdentifier>;
    auto PublishBalance() const noexcept -> void;
    auto ReorgTo(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        const node::HeaderOracle& headers,
        const NodeID& balanceNode,
        const crypto::Subchain subchain,
        const SubchainIndex& index,
        const UnallocatedVector<block::Position>& reorg) const noexcept -> bool;
    auto ReserveUTXO(
        const identifier::Nym& spender,
        const Identifier& proposal,
        node::internal::SpendPolicy& policy) const noexcept
        -> std::optional<UTXO>;
    auto SubchainAddElements(
        const SubchainIndex& index,
        const ElementMap& elements) const noexcept -> bool;
    auto SubchainLastIndexed(const SubchainIndex& index) const noexcept
        -> std::optional<Bip32Index>;
    auto SubchainLastScanned(const SubchainIndex& index) const noexcept
        -> block::Position;
    auto SubchainSetLastScanned(
        const SubchainIndex& index,
        const block::Position& position) const noexcept -> bool;

    Wallet(
        const api::Session& api,
        const common::Database& common,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type chain,
        const blockchain::cfilter::Type filter) noexcept;
    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;

private:
    const api::Session& api_;
    const common::Database& common_;
    const storage::lmdb::LMDB& lmdb_;
    mutable wallet::SubchainData subchains_;
    mutable wallet::Proposal proposals_;
    mutable wallet::Output outputs_;
};
}  // namespace opentxs::blockchain::database::implemenation

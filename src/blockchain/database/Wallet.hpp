// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "api/client/blockchain/database/Database.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/protobuf/BlockchainTransactionProposal.pb.h"
#include "util/LMDB.hpp"

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

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Output;
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionOutput;
class BlockchainTransactionProposal;
}  // namespace proto

class Identifier;
}  // namespace opentxs

namespace opentxs::blockchain::database
{
class Wallet
{
public:
    using Common = api::client::blockchain::database::implementation::Database;
    using Parent = client::internal::WalletDatabase;
    using FilterType = Parent::FilterType;
    using NodeID = Parent::NodeID;
    using pNodeID = Parent::pNodeID;
    using Subchain = Parent::Subchain;
    using ElementID = Parent::ElementID;
    using ElementMap = Parent::ElementMap;
    using Pattern = Parent::Pattern;
    using Patterns = Parent::Patterns;
    using MatchingIndices = Parent::MatchingIndices;
    using UTXO = Parent::UTXO;
    using KeyID = api::client::blockchain::Key;

    auto AddConfirmedTransaction(
        const blockchain::Type chain,
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version,
        const block::Position& block,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept -> bool;
    auto AddOutgoingTransaction(
        const blockchain::Type chain,
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::bitcoin::Transaction& transaction) const noexcept -> bool;
    auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) const noexcept -> bool;
    auto CancelProposal(const Identifier& id) const noexcept -> bool;
    auto CompletedProposals() const noexcept -> std::set<OTIdentifier>;
    auto DeleteProposal(const Identifier& id) const noexcept -> bool;
    auto ForgetProposals(const std::set<OTIdentifier>& ids) const noexcept
        -> bool;
    auto GetBalance() const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner) const noexcept -> Balance;
    auto GetPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> Patterns;
    auto GetUnspentOutputs() const noexcept -> std::vector<UTXO>;
    auto GetUntestedPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ReadView blockID,
        const VersionNumber version) const noexcept -> Patterns;
    auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>;
    auto LoadProposals() const noexcept
        -> std::vector<proto::BlockchainTransactionProposal>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> std::set<OTIdentifier>
    {
        return common_.LookupContact(pubkeyHash);
    }
    auto ReleaseChangeKey(const Identifier& proposal, const KeyID key)
        const noexcept -> bool;
    auto ReorgTo(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const std::vector<block::Position>& reorg) const noexcept -> bool;
    auto ReserveChangeKey(const Identifier& proposal) const noexcept
        -> std::optional<KeyID>;
    auto ReserveUTXO(const Identifier& proposal) const noexcept
        -> std::optional<UTXO>;
    auto SetDefaultFilterType(const FilterType type) const noexcept -> bool;
    auto SubchainAddElements(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ElementMap& elements,
        const VersionNumber version) const noexcept -> bool;
    auto SubchainDropIndex(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> bool;
    auto SubchainIndexVersion(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> VersionNumber;
    auto SubchainLastIndexed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept
        -> std::optional<Bip32Index>;
    auto SubchainLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position;
    auto SubchainLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position;
    auto SubchainMatchBlock(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const MatchingIndices& indices,
        const ReadView blockID,
        const VersionNumber version) const noexcept -> bool;
    auto SubchainSetLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool;
    auto SubchainSetLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool;
    auto TransactionLoadBitcoin(const ReadView txid) const noexcept
        -> std::unique_ptr<block::bitcoin::Transaction>;

    Wallet(
        const api::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const Common& common,
        const blockchain::Type chain) noexcept;

private:
    enum class TxoState : std::uint8_t {
        UnconfirmedNew,
        UnconfirmedSpend,
        ConfirmedNew,
        ConfirmedSpend,
        OrphanedNew,
        OrphanedSpend,
    };

    using SubchainID = Identifier;
    using pSubchainID = OTIdentifier;
    using PatternID = Identifier;
    using pPatternID = OTIdentifier;
    using IDSet = boost::container::flat_set<pPatternID>;
    using PatternMap =
        std::map<pPatternID, std::vector<std::pair<Bip32Index, Space>>>;
    using SubchainPatternIndex = std::map<pSubchainID, IDSet>;
    using SubchainIndexMap = std::map<pSubchainID, VersionNumber>;
    using VersionIndex = std::map<OTIdentifier, VersionNumber>;
    using PositionMap = std::map<OTIdentifier, block::Position>;
    using MatchIndex = std::map<block::pHash, IDSet>;
    using Owners = std::set<OTNymID>;
    using OutputMap = std::map<
        block::bitcoin::Outpoint,
        std::tuple<
            TxoState,
            block::Position,
            proto::BlockchainTransactionOutput>>;
    using OutputPositionIndex =
        std::map<block::Position, std::vector<block::bitcoin::Outpoint>>;
    using OutputStateIndex =
        std::map<TxoState, std::vector<block::bitcoin::Outpoint>>;
    using OutputSubchainIndex =
        std::map<pSubchainID, std::vector<block::bitcoin::Outpoint>>;
    using TransactionBlockMap =
        std::map<block::pTxid, std::vector<block::pHash>>;
    using BlockTransactionMap =
        std::map<block::pHash, std::vector<block::pTxid>>;
    using TransactionHistory =
        std::map<block::Height, std::vector<block::pTxid>>;
    using ProposalMap =
        std::map<OTIdentifier, proto::BlockchainTransactionProposal>;
    using ProposalOutputMap =
        std::map<OTIdentifier, std::vector<block::bitcoin::Outpoint>>;
    using OutputProposalMap = std::map<block::bitcoin::Outpoint, OTIdentifier>;
    using FinishedProposals = std::set<OTIdentifier>;
    using ChangeKeyMap = std::map<OTIdentifier, std::vector<KeyID>>;
    using NymMap = std::map<OTNymID, std::set<block::bitcoin::Outpoint>>;
    using NymBalances = std::map<OTNymID, Balance>;

    const api::Core& api_;
    const api::client::internal::Blockchain& blockchain_;
    const Common& common_;
    const blockchain::Type chain_;
    const FilterType default_filter_type_;
    mutable std::mutex lock_;
    mutable PatternMap patterns_;
    mutable SubchainPatternIndex subchain_pattern_index_;
    mutable SubchainIndexMap subchain_last_indexed_;
    mutable VersionIndex subchain_version_;
    mutable PositionMap subchain_last_scanned_;
    mutable PositionMap subchain_last_processed_;
    mutable MatchIndex match_index_;
    mutable OutputMap outputs_;
    mutable OutputPositionIndex output_positions_;
    mutable OutputStateIndex output_states_;
    mutable OutputSubchainIndex output_subchain_;
    mutable TransactionBlockMap tx_to_block_;
    mutable BlockTransactionMap block_to_tx_;
    mutable TransactionHistory tx_history_;
    mutable ProposalMap proposals_;
    mutable ProposalOutputMap proposal_spent_outpoints_;
    mutable ProposalOutputMap proposal_created_outpoints_;
    mutable OutputProposalMap outpoint_proposal_;
    mutable FinishedProposals finished_proposals_;  // NOTE don't move to lmdb
    mutable ChangeKeyMap change_keys_;
    mutable NymMap nym_map_;

    static auto owns(
        const identifier::Nym& spender,
        proto::BlockchainTransactionOutput) noexcept -> bool;

    auto belongs_to(
        const Lock& lock,
        const block::bitcoin::Outpoint& id,
        const SubchainID& subchain) const noexcept -> bool;
    auto check_proposals(
        const Lock& lock,
        const block::bitcoin::Outpoint& outpoint,
        const block::Position& block,
        const block::Txid& txid) const noexcept -> bool;
    auto effective_position(
        const TxoState state,
        const block::Position& position) const noexcept
        -> const block::Position&;
    auto get_balance(const Lock& lock) const noexcept -> Balance;
    auto get_balance(const Lock& lock, const identifier::Nym& owner)
        const noexcept -> Balance;
    auto get_balances(const Lock& lock) const noexcept -> NymBalances;
    auto get_patterns(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept(false) -> const IDSet&;
    auto get_unspent_outputs(const Lock& lock) const noexcept
        -> std::vector<UTXO>;
    template <typename PatternList>
    auto load_patterns(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain,
        const PatternList& patterns) const noexcept -> Patterns
    {
        auto output = Patterns{};

        for (const auto& patternID : patterns) {
            try {
                for (const auto& [index, pattern] : patterns_.at(patternID)) {
                    output.emplace_back(
                        Pattern{{index, {subchain, balanceNode}}, pattern});
                }
            } catch (...) {
            }
        }

        return output;
    }
    auto load_proposal(const Lock& lock, const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>;

    auto add_transaction(
        const Lock& lock,
        const blockchain::Type chain,
        const block::Position& block,
        const block::bitcoin::Transaction& transaction) const noexcept -> bool;
    auto associate_outpoint(
        const Lock& lock,
        const block::bitcoin::Outpoint& outpoint,
        const KeyID& key) const noexcept -> bool;
    auto change_state(
        const Lock& lock,
        const block::bitcoin::Outpoint& id,
        const TxoState newState,
        const block::Position newPosition) const noexcept -> bool;
    auto create_state(
        const Lock& lock,
        const Owners& owners,
        const block::bitcoin::Outpoint& id,
        const TxoState state,
        const block::Position position,
        const block::bitcoin::Output& output) const noexcept -> bool;
    auto find_output(const Lock& lock, const block::bitcoin::Outpoint& id)
        const noexcept -> std::optional<OutputMap::iterator>;
    auto pattern_id(const SubchainID& subchain, const Bip32Index index)
        const noexcept -> pPatternID;
    auto print(const Lock& lock) const noexcept -> void;
    auto rollback(
        const Lock& lock,
        const SubchainID& subchain,
        const block::Position& position) const noexcept -> bool;
    auto subchain_index_version(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> VersionNumber;
    auto subchain_version_index(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> pSubchainID;
    auto subchain_id(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> pSubchainID;
    auto subchain_id(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain) const noexcept -> pSubchainID;
};
}  // namespace opentxs::blockchain::database

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "blockchain/database/Wallet.hpp"  // IWYU pragma: associated

#include <iosfwd>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/BlockchainTransactionProposal.pb.h"

#define OT_METHOD "opentxs::blockchain::database::Wallet::"

namespace opentxs::blockchain::database
{
Wallet::Wallet(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const Common& common,
    const blockchain::Type chain) noexcept
    : common_(common)
    , subchains_(api)
    , proposals_()
    , transactions_(api, blockchain, common_)
    , outputs_(api, blockchain, chain, subchains_, proposals_, transactions_)
{
}

auto Wallet::AddConfirmedTransaction(
    const blockchain::Type chain,
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version,
    const block::Position& block,
    const std::size_t blockIndex,
    const std::vector<std::uint32_t> outputIndices,
    const block::bitcoin::Transaction& original) const noexcept -> bool
{
    const auto id = subchains_.GetID(balanceNode, subchain, type, version);

    return outputs_.AddConfirmedTransaction(
        balanceNode, id, block, blockIndex, outputIndices, original);
}

auto Wallet::AddOutgoingTransaction(
    const blockchain::Type chain,
    const Identifier& proposalID,
    const proto::BlockchainTransactionProposal& proposal,
    const block::bitcoin::Transaction& transaction) const noexcept -> bool
{
    return outputs_.AddOutgoingTransaction(
        chain, proposalID, proposal, transaction);
}

auto Wallet::AddProposal(
    const Identifier& id,
    const proto::BlockchainTransactionProposal& tx) const noexcept -> bool
{
    return proposals_.AddProposal(id, tx);
}

auto Wallet::CancelProposal(const Identifier& id) const noexcept -> bool
{
    return outputs_.CancelProposal(id);
}

auto Wallet::CompletedProposals() const noexcept -> std::set<OTIdentifier>
{
    return proposals_.CompletedProposals();
}

auto Wallet::ForgetProposals(const std::set<OTIdentifier>& ids) const noexcept
    -> bool
{
    return proposals_.ForgetProposals(ids);
}

auto Wallet::GetBalance() const noexcept -> Balance
{
    return outputs_.GetBalance();
}

auto Wallet::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    return outputs_.GetBalance(owner);
}

auto Wallet::GetBalance(const identifier::Nym& owner, const NodeID& node)
    const noexcept -> Balance
{
    return outputs_.GetBalance(owner, node);
}

auto Wallet::GetOutputs(State type) const noexcept -> std::vector<UTXO>
{
    return outputs_.GetOutputs(type);
}

auto Wallet::GetOutputs(const identifier::Nym& owner, State type) const noexcept
    -> std::vector<UTXO>
{
    return outputs_.GetOutputs(owner, type);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const Identifier& node,
    State type) const noexcept -> std::vector<UTXO>
{
    return outputs_.GetOutputs(owner, node, type);
}

auto Wallet::GetPatterns(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> Patterns
{
    return subchains_.GetPatterns(balanceNode, subchain, type, version);
}

auto Wallet::GetUnspentOutputs() const noexcept -> std::vector<UTXO>
{
    return outputs_.GetUnspentOutputs();
}

auto Wallet::GetUnspentOutputs(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> std::vector<UTXO>
{
    const auto id = subchains_.GetID(balanceNode, subchain, type, version);

    return outputs_.GetUnspentOutputs(id);
}

auto Wallet::GetUntestedPatterns(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const ReadView blockID,
    const VersionNumber version) const noexcept -> Patterns
{
    return subchains_.GetUntestedPatterns(
        balanceNode, subchain, type, blockID, version);
}

auto Wallet::LoadProposal(const Identifier& id) const noexcept
    -> std::optional<proto::BlockchainTransactionProposal>
{
    return proposals_.LoadProposal(id);
}

auto Wallet::LoadProposals() const noexcept
    -> std::vector<proto::BlockchainTransactionProposal>
{
    return proposals_.LoadProposals();
}

auto Wallet::ReorgTo(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const std::vector<block::Position>& reorg) const noexcept -> bool
{
    if (reorg.empty()) { return true; }

    const auto& oldest = *reorg.crbegin();
    const auto lastGoodHeight = block::Height{oldest.first - 1};
    const auto subchainID = subchains_.GetID(balanceNode, subchain, type);
    auto subchainLock = Lock{subchains_.GetMutex(), std::defer_lock};
    auto outputLock = Lock{outputs_.GetMutex(), std::defer_lock};
    std::lock(outputLock, subchainLock);

    try {
        if (subchains_.Reorg(subchainLock, subchainID, lastGoodHeight)) {
            return true;
        }
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        OT_FAIL;
    }

    for (const auto& position : reorg) {
        if (false == outputs_.Rollback(outputLock, subchainID, position)) {
            return false;
        }
    }

    return true;
}

auto Wallet::ReserveUTXO(
    const identifier::Nym& spender,
    const Identifier& id,
    const Spend policy) const noexcept -> std::optional<UTXO>
{
    if (false == proposals_.Exists(id)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Proposal does not exist").Flush();

        return std::nullopt;
    }

    return outputs_.ReserveUTXO(spender, id, policy);
}

auto Wallet::SetDefaultFilterType(const FilterType type) const noexcept -> bool
{
    return subchains_.SetDefaultFilterType(type);
}

auto Wallet::SubchainAddElements(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const ElementMap& elements,
    const VersionNumber version) const noexcept -> bool
{
    return subchains_.SubchainAddElements(
        balanceNode, subchain, type, elements, version);
}

auto Wallet::SubchainDropIndex(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> bool
{
    return subchains_.SubchainDropIndex(balanceNode, subchain, type, version);
}

auto Wallet::SubchainIndexVersion(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> VersionNumber
{
    return subchains_.SubchainIndexVersion(balanceNode, subchain, type);
}

auto Wallet::SubchainLastIndexed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> std::optional<Bip32Index>
{
    return subchains_.SubchainLastIndexed(balanceNode, subchain, type, version);
}

auto Wallet::SubchainLastProcessed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> block::Position
{
    return subchains_.SubchainLastProcessed(balanceNode, subchain, type);
}

auto Wallet::SubchainLastScanned(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> block::Position
{
    return subchains_.SubchainLastScanned(balanceNode, subchain, type);
}

auto Wallet::SubchainSetLastProcessed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const block::Position& position) const noexcept -> bool
{
    return subchains_.SubchainSetLastProcessed(
        balanceNode, subchain, type, position);
}

auto Wallet::SubchainMatchBlock(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const MatchingIndices& indices,
    const ReadView blockID,
    const VersionNumber version) const noexcept -> bool
{
    return subchains_.SubchainMatchBlock(
        balanceNode, subchain, type, indices, blockID, version);
}

auto Wallet::SubchainSetLastScanned(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const block::Position& position) const noexcept -> bool
{
    return subchains_.SubchainSetLastScanned(
        balanceNode, subchain, type, position);
}

auto Wallet::TransactionLoadBitcoin(const ReadView txid) const noexcept
    -> std::unique_ptr<block::bitcoin::Transaction>
{
    return transactions_.TransactionLoadBitcoin(txid);
}
}  // namespace opentxs::blockchain::database

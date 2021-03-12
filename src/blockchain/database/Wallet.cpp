// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "blockchain/database/Wallet.hpp"  // IWYU pragma: associated

#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <iosfwd>
#include <iterator>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include "internal/api/client/Client.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/blockchain/BalanceNode.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"
#include "opentxs/protobuf/BlockchainTransactionProposal.pb.h"
#include "opentxs/protobuf/BlockchainWalletKey.pb.h"
#include "util/Container.hpp"

#define OT_METHOD "opentxs::blockchain::database::Wallet::"

template <typename Key>
auto delete_from_vector(std::vector<Key>& vector, const Key& key) noexcept
    -> bool
{
    auto found{false};

    for (auto i{vector.begin()}; i != vector.end();) {
        if (key == *i) {
            i = vector.erase(i);
            found = true;
        } else {
            ++i;
        }
    }

    return found;
}

namespace opentxs::blockchain::database
{
Wallet::Wallet(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const Common& common,
    const blockchain::Type chain) noexcept
    : api_(api)
    , blockchain_(blockchain)
    , common_(common)
    , chain_(chain)
    , default_filter_type_()
    , lock_()
    , patterns_()
    , subchain_pattern_index_()
    , subchain_last_indexed_()
    , subchain_version_()
    , subchain_last_scanned_()
    , subchain_last_processed_()
    , match_index_()
    , outputs_()
    , output_positions_()
    , output_states_()
    , output_subchain_()
    , tx_to_block_()
    , block_to_tx_()
    , tx_history_()
    , proposals_()
    , proposal_spent_outpoints_()
    , proposal_created_outpoints_()
    , outpoint_proposal_()
    , finished_proposals_()
    , change_keys_()
    , nym_map_()
{
    // TODO persist default_filter_type_ and reindex various tables
    // if the type provided by the filter oracle has changed
}

auto Wallet::add_transaction(
    const Lock& lock,
    const blockchain::Type chain,
    const block::Position& block,
    const block::bitcoin::Transaction& transaction) const noexcept -> bool
{
    const auto& [height, blockHash] = block;

    {
        auto& index = tx_to_block_[transaction.ID()];
        index.emplace_back(blockHash);
        dedup(index);
    }

    {
        auto& index = block_to_tx_[blockHash];
        index.emplace_back(transaction.ID());
        dedup(index);
    }

    {
        auto& index = tx_history_[height];
        index.emplace_back(transaction.ID());
        dedup(index);
    }

    const auto reason =
        api_.Factory().PasswordPrompt("Save a received blockchain transaction");

    return blockchain_.ProcessTransaction(chain, transaction, reason);
}

auto Wallet::AddConfirmedTransaction(
    const blockchain::Type chain,
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version,
    const block::Position& block,
    const std::vector<std::uint32_t> outputIndices,
    const block::bitcoin::Transaction& original) const noexcept -> bool
{
    Lock lock(lock_);
    auto pCopy = original.clone();

    OT_ASSERT(pCopy);

    auto& copy = *pCopy;
    auto inputIndex = int{-1};

    for (const auto& input : copy.Inputs()) {
        const auto& outpoint = input.PreviousOutput();
        ++inputIndex;

        if (false == check_proposals(lock, outpoint, block, copy.ID())) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error updating proposals")
                .Flush();

            return false;
        }

        if (auto out = find_output(lock, outpoint); out.has_value()) {
            const auto& proto = std::get<2>(out.value()->second);

            if (!copy.AssociatePreviousOutput(blockchain_, inputIndex, proto)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error associating previous output to input")
                    .Flush();

                return false;
            }

            if (false ==
                change_state(lock, outpoint, TxoState::ConfirmedSpend, block)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error updating consumed output state")
                    .Flush();

                return false;
            }
        }

        // NOTE consider the case of parallel chain scanning where one
        // transaction spends inputs that belong to two different subchains.
        // The first subchain to find the transaction will recognize the inputs
        // belonging to itself but might miss the inputs belonging to the other
        // subchain if the other subchain's scanning process has not yet
        // discovered those outputs.
        // This is fine. The other scanning process will parse this transaction
        // again and at that point all inputs will be recognized. The only
        // impact is that net balance change of the transaction will
        // underestimated temporarily until scanning is complete for all
        // subchains.
    }

    for (const auto index : outputIndices) {
        const auto outpoint =
            block::bitcoin::Outpoint{copy.ID().Bytes(), index};
        const auto& output = copy.Outputs().at(index);

        OT_ASSERT((0 < output.Keys().size()));

        const auto owners = [&] {
            auto val = Owners{};

            for (const auto& key : output.Keys()) {
                val.emplace(blockchain_.Owner(key));
            }

            return val;
        }();

        OT_ASSERT(0 < owners.size());

        if (auto out = find_output(lock, outpoint); out.has_value()) {
            // TODO is is possible the owners need to be changed here?

            if (false ==
                change_state(lock, outpoint, TxoState::ConfirmedNew, block)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error updating created output state")
                    .Flush();

                return false;
            }
        } else {
            if (false == create_state(
                             lock,
                             owners,
                             outpoint,
                             TxoState::ConfirmedNew,
                             block,
                             output)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error created new output state")
                    .Flush();

                return false;
            }
        }

        {
            auto& vector = output_subchain_[subchain_id(
                balanceNode, subchain, type, version)];
            vector.emplace_back(outpoint);
            dedup(vector);
        }
    }

    if (false == add_transaction(lock, chain, block, copy)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error adding transaction to database")
            .Flush();

        return false;
    }

    print(lock);
    blockchain_.UpdateBalance(chain_, get_balance(lock));

    for (const auto& [nym, balance] : get_balances(lock)) {
        blockchain_.UpdateBalance(nym, chain_, balance);
    }

    return true;
}

auto Wallet::AddOutgoingTransaction(
    const blockchain::Type chain,
    const Identifier& proposalID,
    const proto::BlockchainTransactionProposal& proposal,
    const block::bitcoin::Transaction& transaction) const noexcept -> bool
{
    static const auto block = make_blank<block::Position>::value(api_);
    Lock lock(lock_);

    for (const auto& input : transaction.Inputs()) {
        const auto& outpoint = input.PreviousOutput();

        try {
            if (proposalID != outpoint_proposal_.at(outpoint)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect proposal ID")
                    .Flush();

                return false;
            }
        } catch (...) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Input spending")(
                outpoint.str())(" not registered with a proposal")
                .Flush();

            return false;
        }
    }

    auto index{-1};
    auto& pending = proposal_created_outpoints_[proposalID];

    for (const auto& output : transaction.Outputs()) {
        ++index;

        if (0 == output.Keys().size()) {
            LogTrace(OT_METHOD)(__FUNCTION__)(": output ")(index)(
                " belongs to someone else")
                .Flush();

            continue;
        } else {
            LogTrace(OT_METHOD)(__FUNCTION__)(": output ")(index)(
                " belongs to me")
                .Flush();
        }

        const auto outpoint = block::bitcoin::Outpoint{
            transaction.ID().Bytes(), static_cast<std::uint32_t>(index)};
        pending.emplace_back(outpoint);

        if (auto out = find_output(lock, outpoint); out.has_value()) {
            if (false ==
                change_state(lock, outpoint, TxoState::UnconfirmedNew, block)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error updating created output state")
                    .Flush();

                return false;
            }
        } else {
            const auto owners = [&] {
                auto val = Owners{};

                for (const auto& key : output.Keys()) {
                    val.emplace(blockchain_.Owner(key));
                }

                return val;
            }();

            if (false == create_state(
                             lock,
                             owners,
                             outpoint,
                             TxoState::UnconfirmedNew,
                             block,
                             output)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error creating new output state")
                    .Flush();

                return false;
            }
        }

        for (const auto& key : output.Keys()) {
            if (false == associate_outpoint(lock, outpoint, key)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error associating output to subchain")
                    .Flush();

                return false;
            }
        }
    }

    if (false == add_transaction(lock, chain, block, transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error adding transaction to database")
            .Flush();

        return false;
    }

    dedup(pending);
    print(lock);
    blockchain_.UpdateBalance(chain_, get_balance(lock));

    for (const auto& [nym, balance] : get_balances(lock)) {
        blockchain_.UpdateBalance(nym, chain_, balance);
    }

    return true;
}

auto Wallet::AddProposal(
    const Identifier& id,
    const proto::BlockchainTransactionProposal& tx) const noexcept -> bool
{
    Lock lock{lock_};
    proposals_[id] = tx;

    return true;
}

auto Wallet::associate_outpoint(
    const Lock& lock,
    const block::bitcoin::Outpoint& outpoint,
    const KeyID& key) const noexcept -> bool
{
    const auto& [nodeID, subchain, index] = key;
    const auto subchainID =
        subchain_id(lock, api_.Factory().Identifier(nodeID), subchain);
    auto& vector = output_subchain_[subchainID];
    vector.emplace_back(outpoint);
    dedup(vector);

    return true;
}

auto Wallet::belongs_to(
    const Lock& lock,
    const block::bitcoin::Outpoint& id,
    const SubchainID& subchain) const noexcept -> bool
{
    const auto it1 = output_subchain_.find(subchain);

    if (output_subchain_.cend() == it1) { return false; }

    const auto& vector = it1->second;

    for (const auto& outpoint : vector) {
        if (id == outpoint) { return true; }
    }

    return false;
}

auto Wallet::change_state(
    const Lock& lock,
    const block::bitcoin::Outpoint& id,
    const TxoState newState,
    const block::Position newPosition) const noexcept -> bool
{
    auto itOutput = outputs_.find(id);

    if (outputs_.end() == itOutput) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Outpoint does not exist in db")
            .Flush();

        return false;
    }

    auto& [outpointState, outpointPosition, data] = itOutput->second;

    if (false == delete_from_vector(output_states_[outpointState], id)) {
        // Repair database inconsistency

        for (auto& [state, vector] : output_states_) {
            if (state == outpointState) { continue; }

            delete_from_vector(vector, id);
        }
    }

    {
        outpointState = newState;
        auto& vector = output_states_[outpointState];
        vector.emplace_back(id);
        dedup(vector);
    }

    if (false == delete_from_vector(output_positions_[outpointPosition], id)) {
        // Repair database inconsistency

        for (auto& [position, vector] : output_positions_) {
            if (position == outpointPosition) { continue; }

            delete_from_vector(vector, id);
        }
    }

    {
        outpointPosition = effective_position(newState, newPosition);
        auto& vector = output_positions_[outpointPosition];
        vector.emplace_back(id);
        dedup(vector);
    }

    return true;
}

auto Wallet::check_proposals(
    const Lock& lock,
    const block::bitcoin::Outpoint& outpoint,
    const block::Position& block,
    const block::Txid& txid) const noexcept -> bool

{
    if (0 == outpoint_proposal_.count(outpoint)) { return true; }

    auto proposalID{outpoint_proposal_.at(outpoint)};

    if (0 < proposal_created_outpoints_.count(proposalID)) {
        auto& created = proposal_created_outpoints_.at(proposalID);

        for (const auto& newOutpoint : created) {
            const auto rhs = api_.Factory().Data(newOutpoint.Txid());

            if (txid != rhs) {
                const auto changed =
                    change_state(lock, outpoint, TxoState::OrphanedNew, block);

                if (false == changed) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failed to update txo state")
                        .Flush();

                    return false;
                }
            }
        }

        proposal_created_outpoints_.erase(proposalID);
    }

    if (0 < proposal_spent_outpoints_.count(proposalID)) {
        auto& spent = proposal_spent_outpoints_.at(proposalID);

        for (const auto& spentOutpoint : spent) {
            outpoint_proposal_.erase(spentOutpoint);
        }

        proposal_spent_outpoints_.erase(proposalID);
    }

    proposals_.erase(proposalID);
    finished_proposals_.emplace(std::move(proposalID));

    return true;
}

auto Wallet::create_state(
    const Lock& lock,
    const Owners& owners,
    const block::bitcoin::Outpoint& id,
    const TxoState state,
    const block::Position position,
    const block::bitcoin::Output& output) const noexcept -> bool
{
    if (0 < outputs_.count(id)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Outpoint already exists in db")
            .Flush();

        return false;
    }

    auto data = block::bitcoin::Output::SerializeType{};

    if (false == output.Serialize(blockchain_, data)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize output")
            .Flush();

        return false;
    }

    const auto& effectivePosition = effective_position(state, position);
    outputs_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(state, effectivePosition, std::move(data)));

    {
        auto& vector = output_states_[state];
        vector.emplace_back(id);
        dedup(vector);
    }

    {
        auto& vector = output_positions_[effectivePosition];
        vector.emplace_back(id);
        dedup(vector);
    }

    for (const auto& nym : owners) { nym_map_[nym].emplace(id); }

    return true;
}

auto Wallet::CancelProposal(const Identifier& id) const noexcept -> bool
{
    static const auto blank = api_.Factory().Identifier();

    Lock lock(lock_);

    {
        auto& available = change_keys_[blank];

        try {
            auto& used = change_keys_.at(id);
            std::move(
                std::begin(used),
                std::end(used),
                std::back_inserter(available));
            change_keys_.erase(id);
        } catch (...) {
        }
    }

    auto& reserved = proposal_spent_outpoints_[id];
    auto& created = proposal_created_outpoints_[id];
    auto& outgoing = output_states_[TxoState::UnconfirmedSpend];
    auto& available = output_states_[TxoState::ConfirmedNew];
    auto& pending = output_states_[TxoState::UnconfirmedNew];
    auto& orphaned = output_states_[TxoState::OrphanedNew];

    for (const auto& outpoint : reserved) {
        auto& [state, position, data] = outputs_.at(outpoint);
        state = TxoState::ConfirmedNew;
        delete_from_vector(outgoing, outpoint);
        available.emplace_back(outpoint);
        dedup(available);
        outpoint_proposal_.erase(outpoint);
    }

    for (const auto& outpoint : created) {
        auto& [state, position, data] = outputs_.at(outpoint);
        state = TxoState::OrphanedNew;
        delete_from_vector(pending, outpoint);
        orphaned.emplace_back(outpoint);
        dedup(orphaned);
    }

    proposal_spent_outpoints_.erase(id);
    proposal_created_outpoints_.erase(id);
    proposals_.erase(id);

    return true;
}

auto Wallet::CompletedProposals() const noexcept -> std::set<OTIdentifier>
{
    Lock lock(lock_);

    return finished_proposals_;
}

auto Wallet::DeleteProposal(const Identifier& id) const noexcept -> bool
{
    Lock lock{lock_};
    proposals_.erase(id);

    return true;
}

auto Wallet::effective_position(
    const TxoState state,
    const block::Position& position) const noexcept -> const block::Position&
{
    static const auto blankPosition = make_blank<block::Position>::value(api_);

    return ((TxoState::UnconfirmedNew == state) ||
            (TxoState::UnconfirmedSpend == state))
               ? blankPosition
               : position;
}

auto Wallet::find_output(const Lock& lock, const block::bitcoin::Outpoint& id)
    const noexcept -> std::optional<OutputMap::iterator>
{
    auto result = outputs_.find(id);

    if (outputs_.end() == result) {

        return {};
    } else {

        return result;
    }
}

auto Wallet::ForgetProposals(const std::set<OTIdentifier>& ids) const noexcept
    -> bool
{
    Lock lock(lock_);

    for (const auto& id : ids) { finished_proposals_.erase(id); }

    return true;
}

auto Wallet::get_balance(const Lock& lock) const noexcept -> Balance
{
    const auto nym = api_.Factory().NymID();

    return get_balance(lock, nym);
}

auto Wallet::get_balance(const Lock&, const identifier::Nym& owner)
    const noexcept -> Balance
{
    auto output = Balance{};
    auto& [confirmed, unconfirmed] = output;
    auto cb = [&](const auto previous, const auto& outpoint) -> auto
    {
        const auto& [state, position, data] = outputs_.at(outpoint);

        if (owner.empty() || (0 < nym_map_[owner].count(outpoint))) {

            return previous + data.value();
        } else {

            return previous;
        }
    };
    const auto& confirmedNew = output_states_[TxoState::ConfirmedNew];
    const auto& unconfirmedSpend = output_states_[TxoState::UnconfirmedSpend];
    const auto unconfirmedSpendTotal = std::accumulate(
        std::begin(unconfirmedSpend),
        std::end(unconfirmedSpend),
        std::uint64_t{0},
        cb);
    confirmed = std::accumulate(
                    std::begin(confirmedNew),
                    std::end(confirmedNew),
                    std::uint64_t{0},
                    cb) +
                unconfirmedSpendTotal;
    const auto& unconfirmedNew = output_states_[TxoState::UnconfirmedNew];
    unconfirmed = std::accumulate(
                      std::begin(unconfirmedNew),
                      std::end(unconfirmedNew),
                      confirmed,
                      cb) -
                  unconfirmedSpendTotal;

    return output;
}

auto Wallet::get_balances(const Lock& lock) const noexcept -> NymBalances
{
    auto output = NymBalances{};

    for (const auto& [nym, outpoints] : nym_map_) {
        output[nym] = get_balance(lock, nym);
    }

    return output;
}

auto Wallet::get_patterns(
    const Lock& lock,
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept(false) -> const IDSet&
{
    return subchain_pattern_index_.at(
        subchain_id(balanceNode, subchain, type, version));
}

auto Wallet::GetBalance() const noexcept -> Balance
{
    Lock lock(lock_);

    return get_balance(lock);
}

auto Wallet::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    Lock lock(lock_);

    return get_balance(lock, owner);
}

auto Wallet::GetPatterns(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> Patterns
{
    Lock lock(lock_);

    try {
        const auto& patterns =
            get_patterns(lock, balanceNode, subchain, type, version);

        return load_patterns(lock, balanceNode, subchain, patterns);
    } catch (...) {

        return {};
    }
}

auto Wallet::GetUnspentOutputs() const noexcept -> std::vector<UTXO>
{
    Lock lock(lock_);

    return get_unspent_outputs(lock);
}

auto Wallet::get_unspent_outputs(const Lock& lock) const noexcept
    -> std::vector<UTXO>
{
    auto retrieve = std::vector<block::bitcoin::Outpoint>{};

    for (const auto& outpoint : output_states_[TxoState::UnconfirmedNew]) {
        retrieve.emplace_back(outpoint);
    }

    for (const auto& outpoint : output_states_[TxoState::ConfirmedNew]) {
        retrieve.emplace_back(outpoint);
    }

    for (const auto& outpoint : output_states_[TxoState::UnconfirmedSpend]) {
        retrieve.emplace_back(outpoint);
    }

    dedup(retrieve);
    auto output = std::vector<UTXO>{};

    for (const auto& outpoint : retrieve) {
        const auto& [state, position, data] = outputs_.at(outpoint);
        output.emplace_back(outpoint, data);
    }

    return output;
}

auto Wallet::GetUntestedPatterns(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const ReadView blockID,
    const VersionNumber version) const noexcept -> Patterns
{
    Lock lock(lock_);

    try {
        const auto& allPatterns =
            get_patterns(lock, balanceNode, subchain, type, version);
        auto effectiveIDs = std::vector<pPatternID>{};

        try {
            const auto& matchedPatterns =
                match_index_.at(api_.Factory().Data(blockID));
            std::set_difference(
                std::begin(allPatterns),
                std::end(allPatterns),
                std::begin(matchedPatterns),
                std::end(matchedPatterns),
                std::back_inserter(effectiveIDs));
        } catch (...) {

            return load_patterns(lock, balanceNode, subchain, allPatterns);
        }

        return load_patterns(lock, balanceNode, subchain, effectiveIDs);
    } catch (...) {

        return {};
    }
}

auto Wallet::LoadProposal(const Identifier& id) const noexcept
    -> std::optional<proto::BlockchainTransactionProposal>
{
    Lock lock(lock_);

    return load_proposal(lock, id);
}

auto Wallet::load_proposal(const Lock& lock, const Identifier& id)
    const noexcept -> std::optional<proto::BlockchainTransactionProposal>
{
    auto it = proposals_.find(id);

    if (proposals_.end() == it) { return std::nullopt; }

    return it->second;
}

auto Wallet::LoadProposals() const noexcept
    -> std::vector<proto::BlockchainTransactionProposal>
{
    auto output = std::vector<proto::BlockchainTransactionProposal>{};
    Lock lock{lock_};
    std::transform(
        std::begin(proposals_),
        std::end(proposals_),
        std::back_inserter(output),
        [](const auto& in) -> auto { return in.second; });

    return output;
}

auto Wallet::owns(
    const identifier::Nym& spender,
    proto::BlockchainTransactionOutput output) noexcept -> bool
{
    const auto id = spender.str();

    for (const auto& key : output.key()) {
        if (key.nym() == id) { return true; }
    }

    if (0 == output.key_size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No keys").Flush();
    }

    return false;
}

auto Wallet::pattern_id(const SubchainID& subchain, const Bip32Index index)
    const noexcept -> pPatternID
{
    auto preimage = OTData{subchain};
    preimage->Concatenate(&index, sizeof(index));
    auto output = api_.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}

auto Wallet::print(const Lock&) const noexcept -> void
{
    struct Output {
        std::stringstream text_{};
        std::size_t total_{};
    };
    auto output = std::map<TxoState, Output>{};

    for (const auto& data : outputs_) {
        const auto& outpoint = data.first;
        const auto& [state, position, proto] = data.second;
        auto& out = output[state];
        out.text_ << "\n * " << outpoint.str() << ' ';
        out.text_ << " value: " << std::to_string(proto.value());
        out.total_ += proto.value();
        const auto pScript = factory::BitcoinScript(chain_, proto.script());

        OT_ASSERT(pScript);

        const auto& script = *pScript;
        out.text_ << ", type: ";
        using Pattern = block::bitcoin::Script::Pattern;

        switch (script.Type()) {
            case Pattern::PayToMultisig: {
                out.text_ << "P2MS";
            } break;
            case Pattern::PayToPubkey: {
                out.text_ << "P2PK";
            } break;
            case Pattern::PayToPubkeyHash: {
                out.text_ << "P2PKH";
            } break;
            case Pattern::PayToScriptHash: {
                out.text_ << "P2SH";
            } break;
            default: {
                out.text_ << "unknown";
            }
        }
    }

    const auto& unconfirmed = output[TxoState::UnconfirmedNew];
    const auto& confirmed = output[TxoState::ConfirmedNew];
    const auto& pending = output[TxoState::UnconfirmedSpend];
    const auto& spent = output[TxoState::ConfirmedSpend];
    LogTrace(OT_METHOD)(__FUNCTION__)(": Instance ")(api_.Instance())(
        " TXO database contents:")
        .Flush();
    LogTrace(OT_METHOD)(__FUNCTION__)(": Unconfirmed available value: ")(
        unconfirmed.total_)(unconfirmed.text_.str())
        .Flush();
    LogTrace(OT_METHOD)(__FUNCTION__)(": Confirmed available value: ")(
        confirmed.total_)(confirmed.text_.str())
        .Flush();
    LogTrace(OT_METHOD)(__FUNCTION__)(": Unconfirmed spent value: ")(
        pending.total_)(pending.text_.str())
        .Flush();
    LogTrace(OT_METHOD)(__FUNCTION__)(": Confirmed spent value: ")(
        spent.total_)(spent.text_.str())
        .Flush();
}

auto Wallet::ReleaseChangeKey(const Identifier& proposal, const KeyID key)
    const noexcept -> bool
{
    static const auto blank = api_.Factory().Identifier();

    Lock lock(lock_);
    auto& available = change_keys_[blank];
    auto& used = change_keys_[proposal];

    for (auto it{used.begin()}; it != used.end();) {
        if (key == *it) {
            available.emplace_back(std::move(*it));
            it = used.erase(it);
        } else {
            ++it;
        }
    }

    return true;
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
    Lock lock(lock_);
    const auto subchainID = subchain_version_index(balanceNode, subchain, type);

    try {
        auto& scanned = subchain_last_scanned_.at(subchainID);
        const auto& currentHeight = scanned.first;

        if (currentHeight < lastGoodHeight) {
            // noop
        } else if (currentHeight > lastGoodHeight) {
            scanned.first = lastGoodHeight;
        } else {
            scanned.first = std::min<block::Height>(lastGoodHeight - 1, 0);
        }

    } catch (...) {
        OT_FAIL;
    }

    try {
        auto& processed = subchain_last_processed_.at(subchainID);
        const auto& currentHeight = processed.first;

        if (currentHeight < lastGoodHeight) { return true; }

        for (const auto& position : reorg) {
            if (false == rollback(lock, subchainID, position)) { return false; }
        }
    } catch (...) {
        OT_FAIL;
    }

    return true;
}

auto Wallet::ReserveChangeKey(const Identifier& proposal) const noexcept
    -> std::optional<KeyID>
{
    static const auto blank = api_.Factory().Identifier();

    Lock lock(lock_);
    auto& available = change_keys_[blank];

    if (0 < available.size()) {
        auto it = available.begin();
        auto output{*it};
        auto& used = change_keys_[proposal];
        used.emplace_back(output);
        available.erase(it);

        return std::make_optional<KeyID>(std::move(output));
    }

    try {
        const auto& data = proposals_.at(proposal);
        const auto nym = api_.Factory().NymID(data.initiator());

        try {
            const auto& account = blockchain_.Account(nym, chain_);
            const auto reason =
                api_.Factory().PasswordPrompt("Send a blockchain transaction");
            const auto& node = account.GetNextChangeKey(reason);
            auto output{node.KeyID()};
            auto& used = change_keys_[proposal];
            used.emplace_back(output);

            return std::make_optional<KeyID>(std::move(output));
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

            return std::nullopt;
        }
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid proposal").Flush();

        return std::nullopt;
    }
}

auto Wallet::ReserveUTXO(const Identifier& id) const noexcept
    -> std::optional<UTXO>
{
    // TODO optionally spend unconfirmed
    // TODO implement smarter selection algorithms
    Lock lock(lock_);
    const auto proposal = load_proposal(lock, id);

    if (false == proposal.has_value()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Proposal does not exist").Flush();

        return std::nullopt;
    }

    const auto spender = api_.Factory().NymID(proposal.value().initiator());

    try {
        auto& confirmed = output_states_.at(TxoState::ConfirmedNew);

        if (confirmed.empty()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": No spendable outputs")
                .Flush();

            return std::nullopt;
        }

        for (auto outpoint : confirmed) {  // intentional copy
            auto& [state, position, serialized] = outputs_.at(outpoint);

            if (owns(spender, serialized)) {
                auto output = std::make_optional<UTXO>(outpoint, serialized);
                state = TxoState::UnconfirmedSpend;
                output_states_[TxoState::UnconfirmedSpend].emplace_back(
                    outpoint);
                auto& reserved = proposal_spent_outpoints_[id];
                reserved.emplace_back(outpoint);
                dedup(reserved);
                LogVerbose(OT_METHOD)(__FUNCTION__)(": Reserving output ")(
                    outpoint.str())
                    .Flush();
                outpoint_proposal_.emplace(outpoint, id);
                delete_from_vector(confirmed, outpoint);  // NOTE outpoint would
                                                          // be invalidated if
                                                          // it was a reference
                                                          // instead of a copy

                return output;
            }
        }

        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No spendable outputs for specified nym")
            .Flush();

        return std::nullopt;
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No spendable outputs").Flush();

        return std::nullopt;
    }
}

auto Wallet::rollback(
    const Lock& lock,
    const SubchainID& subchain,
    const block::Position& position) const noexcept -> bool
{
    // TODO rebroadcast transactions which have become unconfirmed
    auto outpoints = std::vector<block::bitcoin::Outpoint>{};

    for (const auto& outpoint : output_positions_[position]) {
        if (belongs_to(lock, outpoint, subchain)) {
            outpoints.emplace_back(outpoint);
        }
    }

    dedup(outpoints);
    auto& history = tx_history_[position.first];

    for (const auto& id : outpoints) {
        auto& [opState, opPosition, data] = outputs_.at(id);
        auto change{true};
        auto newState = TxoState{};

        switch (opState) {
            case TxoState::ConfirmedNew:
            case TxoState::OrphanedNew: {
                newState = TxoState::UnconfirmedNew;
            } break;
            case TxoState::ConfirmedSpend:
            case TxoState::OrphanedSpend: {
                newState = TxoState::UnconfirmedSpend;
            } break;
            default: {
                change = false;
            }
        }

        if (change && (false == change_state(lock, id, newState, position))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to update output state")
                .Flush();

            return false;
        }

        const auto& txid = api_.Factory().Data(id.Txid());

        if (false == delete_from_vector(history, txid)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to update transaction history")
                .Flush();

            return false;
        }
    }

    return true;
}

auto Wallet::SetDefaultFilterType(const FilterType type) const noexcept -> bool
{
    const_cast<FilterType&>(default_filter_type_) = type;

    return true;
}

auto Wallet::SubchainAddElements(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const ElementMap& elements,
    const VersionNumber version) const noexcept -> bool
{
    Lock lock(lock_);
    subchain_version_[subchain_version_index(balanceNode, subchain, type)] =
        version;
    auto subchainID = subchain_id(balanceNode, subchain, type, version);
    auto newIndices = std::vector<OTIdentifier>{};
    auto highest = Bip32Index{};

    for (const auto& [index, patterns] : elements) {
        auto patternID = pattern_id(subchainID, index);
        auto& vector = patterns_[patternID];
        newIndices.emplace_back(std::move(patternID));
        highest = std::max(highest, index);

        for (const auto& pattern : patterns) {
            vector.emplace_back(index, pattern);
        }
    }

    subchain_last_indexed_[subchainID] = highest;
    auto& index = subchain_pattern_index_[subchainID];

    for (auto& id : newIndices) { index.emplace(std::move(id)); }

    return true;
}

auto Wallet::SubchainDropIndex(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> bool
{
    Lock lock(lock_);
    const auto subchainID = subchain_id(balanceNode, subchain, type, version);

    try {
        for (const auto& patternID : subchain_pattern_index_.at(subchainID)) {
            patterns_.erase(patternID);

            for (auto& [block, set] : match_index_) { set.erase(patternID); }
        }
    } catch (...) {
    }

    subchain_pattern_index_.erase(subchainID);
    subchain_last_indexed_.erase(subchainID);
    subchain_version_.erase(
        subchain_version_index(balanceNode, subchain, type));

    return true;
}

auto Wallet::subchain_index_version(
    const Lock& lock,
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> VersionNumber
{
    const auto id = subchain_version_index(balanceNode, subchain, type);

    try {

        return subchain_version_.at(id);
    } catch (...) {

        return 0;
    }
}

auto Wallet::SubchainIndexVersion(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> VersionNumber
{
    Lock lock(lock_);

    return subchain_index_version(lock, balanceNode, subchain, type);
}

auto Wallet::SubchainLastIndexed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> std::optional<Bip32Index>
{
    Lock lock(lock_);
    const auto subchainID = subchain_id(balanceNode, subchain, type, version);

    try {
        return subchain_last_indexed_.at(subchainID);
    } catch (...) {
        return {};
    }
}

auto Wallet::SubchainLastProcessed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> block::Position
{
    Lock lock(lock_);

    try {
        return subchain_last_processed_.at(
            subchain_version_index(balanceNode, subchain, type));
    } catch (...) {
        return make_blank<block::Position>::value(api_);
    }
}

auto Wallet::SubchainLastScanned(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> block::Position
{
    Lock lock(lock_);

    try {
        return subchain_last_scanned_.at(
            subchain_version_index(balanceNode, subchain, type));
    } catch (...) {
        return make_blank<block::Position>::value(api_);
    }
}

auto Wallet::SubchainSetLastProcessed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const block::Position& position) const noexcept -> bool
{
    Lock lock(lock_);
    auto& map = subchain_last_processed_;
    auto id = subchain_version_index(balanceNode, subchain, type);
    auto it = map.find(id);

    if (map.end() == it) {
        map.emplace(std::move(id), position);

        return true;
    } else {
        it->second = position;

        return true;
    }
}

auto Wallet::SubchainMatchBlock(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const MatchingIndices& indices,
    const ReadView blockID,
    const VersionNumber version) const noexcept -> bool
{
    Lock lock(lock_);
    auto& matchSet = match_index_[api_.Factory().Data(blockID)];

    for (const auto& index : indices) {
        matchSet.emplace(pattern_id(
            subchain_id(balanceNode, subchain, type, version), index));
    }

    return true;
}

auto Wallet::SubchainSetLastScanned(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const block::Position& position) const noexcept -> bool
{
    Lock lock(lock_);
    auto& map = subchain_last_scanned_;
    auto id = subchain_version_index(balanceNode, subchain, type);
    auto it = map.find(id);

    if (map.end() == it) {
        map.emplace(std::move(id), position);

        return true;
    } else {
        it->second = position;

        return true;
    }
}

auto Wallet::subchain_version_index(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> pSubchainID
{
    auto preimage = OTData{balanceNode};
    preimage->Concatenate(&subchain, sizeof(subchain));
    preimage->Concatenate(&type, sizeof(type));
    auto output = api_.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}

auto Wallet::subchain_id(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> pSubchainID
{
    auto preimage = OTData{balanceNode};
    preimage->Concatenate(&subchain, sizeof(subchain));
    preimage->Concatenate(&type, sizeof(type));
    preimage->Concatenate(&version, sizeof(version));
    auto output = api_.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}

auto Wallet::subchain_id(
    const Lock& lock,
    const NodeID& nodeID,
    const Subchain subchain) const noexcept -> pSubchainID
{
    return subchain_id(
        nodeID,
        subchain,
        default_filter_type_,
        subchain_index_version(lock, nodeID, subchain, default_filter_type_));
}

auto Wallet::TransactionLoadBitcoin(const ReadView txid) const noexcept
    -> std::unique_ptr<block::bitcoin::Transaction>
{
    const auto serialized = common_.LoadTransaction(txid);

    if (false == serialized.has_value()) { return {}; }

    return factory::BitcoinTransaction(api_, blockchain_, serialized.value());
}
}  // namespace opentxs::blockchain::database

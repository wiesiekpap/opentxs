// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/database/wallet/Output.hpp"  // IWYU pragma: associated

#include <cs_shared_guarded.h>
#include <robin_hood.h>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <numeric>
#include <optional>
#include <shared_mutex>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "blockchain/database/wallet/OutputCache.hpp"
#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/Proposal.hpp"
#include "blockchain/database/wallet/Subchain.hpp"
#include "blockchain/database/wallet/Types.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/node/SpendPolicy.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Inputs.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Outputs.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/blockchain/node/TxoTag.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BlockchainTransactionOutput.pb.h"  // IWYU pragma: keep
#include "util/LMDB.hpp"

namespace opentxs::blockchain::database::wallet
{
struct Output::Imp {
private:
    [[nodiscard]] auto lock_shared() const noexcept
    {
        auto out = cache_.lock_shared();
        out->Populate();

        return out;
    }

    [[nodiscard]] auto lock() noexcept
    {
        auto out = cache_.lock();
        out->Populate();

        return out;
    }

public:
    auto GetBalance() const noexcept -> Balance
    {
        return get_balance(*lock_shared());
    }
    auto GetBalance(const identifier::Nym& owner) const noexcept -> Balance
    {
        if (owner.empty()) { return {}; }

        return get_balance(*lock_shared(), owner);
    }
    auto GetBalance(const identifier::Nym& owner, const NodeID& node)
        const noexcept -> Balance
    {
        if (owner.empty() || node.empty()) { return {}; }

        return get_balance(*lock_shared(), owner, node, nullptr);
    }
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance
    {
        static const auto owner = api_.Factory().NymID();
        static const auto node = api_.Factory().Identifier();

        return get_balance(*lock_shared(), owner, node, &key);
    }
    auto GetOutputs(node::TxoState type, alloc::Resource* alloc) const noexcept
        -> Vector<UTXO>
    {
        if (node::TxoState::Error == type) { return {}; }

        return get_outputs(
            *lock_shared(),
            states(type),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            alloc);
    }
    auto GetOutputs(
        const identifier::Nym& owner,
        node::TxoState type,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>
    {
        if (node::TxoState::Error == type) { return {}; }
        if (owner.empty()) { return {}; }

        return get_outputs(
            *lock_shared(),
            states(type),
            &owner,
            nullptr,
            nullptr,
            nullptr,
            alloc);
    }
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        node::TxoState type,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>
    {
        if (node::TxoState::Error == type) { return {}; }
        if (owner.empty() || node.empty()) { return {}; }

        return get_outputs(
            *lock_shared(),
            states(type),
            &owner,
            &node,
            nullptr,
            nullptr,
            alloc);
    }
    auto GetOutputs(
        const crypto::Key& key,
        node::TxoState type,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>
    {
        if (node::TxoState::Error == type) { return {}; }

        return get_outputs(
            *lock_shared(),
            states(type),
            nullptr,
            nullptr,
            nullptr,
            &key,
            alloc);
    }
    auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<node::TxoTag>
    {
        auto handle = lock_shared();

        if (handle->Exists(output)) {
            const auto& existing = handle->GetOutput(output);

            return existing.Tags();
        } else {

            return {};
        }
    }
    auto GetTransactions() const noexcept -> UnallocatedVector<block::pTxid>
    {
        return translate(GetOutputs(node::TxoState::All, alloc::System()));
    }
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::pTxid>
    {
        return translate(
            GetOutputs(account, node::TxoState::All, alloc::System()));
    }
    auto GetUnconfirmedTransactions() const noexcept
        -> UnallocatedSet<block::pTxid>
    {
        auto out = UnallocatedSet<block::pTxid>{};
        const auto unconfirmed =
            GetOutputs(node::TxoState::UnconfirmedNew, alloc::System());

        for (const auto& [outpoint, output] : unconfirmed) {
            out.emplace(api_.Factory().DataFromBytes(outpoint.Txid()));
        }

        return out;
    }
    auto GetUnspentOutputs(alloc::Resource* alloc) const noexcept
        -> Vector<UTXO>
    {
        static const auto blank = api_.Factory().Identifier();

        return GetUnspentOutputs(blank, alloc);
    }
    auto GetUnspentOutputs(const NodeID& id, alloc::Resource* alloc)
        const noexcept -> Vector<UTXO>
    {
        return get_unspent_outputs(*lock_shared(), id, alloc);
    }
    auto GetWalletHeight() const noexcept -> block::Height
    {
        return lock_shared()->GetHeight();
    }
    auto PublishBalance() const noexcept -> void
    {
        publish_balance(*lock_shared());
    }

    auto AddTransactions(
        const AccountID& account,
        const SubchainID& subchain,
        const node::TxoState consumed,
        const node::TxoState created,
        BatchedMatches&& transactions,
        TXOs& txoCreated,
        TXOs& txoConsumed) noexcept -> bool
    {
        const auto& log = LogTrace();
        auto handle = lock();
        auto& cache = *handle;

        try {
            auto processed = Set<std::shared_ptr<bitcoin::block::Transaction>>{
                transactions.get_allocator()};
            auto tx = lmdb_.TransactionRW();

            for (const auto& [block, blockMatches] : transactions) {
                log(OT_PRETTY_CLASS())("processing block ")(block).Flush();
                add_transactions(
                    log,
                    account,
                    subchain,
                    block,
                    blockMatches,
                    consumed,
                    created,
                    processed,
                    txoCreated,
                    txoConsumed,
                    cache,
                    tx);
            }

            if (false == tx.Finalize(true)) {
                throw std::runtime_error{
                    "Failed to commit database transaction"};
            }

            const auto reason = api_.Factory().PasswordPrompt(
                "Save a received blockchain transaction(s)");
            const auto added =
                api_.Crypto().Blockchain().Internal().ProcessTransactions(
                    chain_, std::move(processed), reason);

            if (false == added) {
                throw std::runtime_error{
                    "Error adding transaction to activity database"};
            }

            // NOTE uncomment this for detailed debugging: cache.Print();
            publish_balance(cache);
            transactions.clear();

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            cache.Clear();

            return false;
        }
    }
    auto AddConfirmedTransactions(
        const NodeID& account,
        const SubchainIndex& subchain,
        BatchedMatches&& transactions,
        TXOs& txoCreated,
        TXOs& txoConsumed) noexcept -> bool
    {
        return AddTransactions(
            account,
            subchain,
            node::TxoState::ConfirmedSpend,
            node::TxoState::ConfirmedNew,
            std::move(transactions),
            txoCreated,
            txoConsumed);
    }
    auto AddMempoolTransaction(
        const AccountID& account,
        const SubchainID& subchain,
        const Vector<std::uint32_t>& outputIndices,
        const bitcoin::block::Transaction& original,
        TXOs& txoCreated) noexcept -> bool
    {
        static const auto block = block::Position{};
        auto txoConsumed = TXOs{};

        return AddTransactions(
            account,
            subchain,
            node::TxoState::UnconfirmedSpend,
            node::TxoState::UnconfirmedNew,
            [&] {
                auto out = BatchedMatches{txoCreated.get_allocator()};
                auto& matches = out[block];
                matches.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(original.ID()),
                    std::forward_as_tuple(outputIndices, original.clone()));

                return out;
            }(),
            txoCreated,
            txoConsumed);
    }
    auto AddOutgoingTransaction(
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const bitcoin::block::Transaction& transaction) noexcept -> bool
    {
        OT_ASSERT(false == transaction.IsGeneration());

        auto handle = lock();
        auto& cache = *handle;
        const auto& api = api_.Crypto().Blockchain();

        try {
            for (const auto& input : transaction.Inputs()) {
                const auto& outpoint = input.PreviousOutput();

                auto found{false};
                lmdb_.Load(
                    output_proposal_, outpoint.Bytes(), [&](const auto bytes) {
                        found = true;

                        if (bytes != proposalID.Bytes()) {
                            throw std::runtime_error{"Incorrect proposal ID"};
                        }
                    });

                if (false == found) {
                    const auto error = UnallocatedCString{
                        "Input spending " + outpoint.str() +
                        " is not registered with a proposal"};

                    throw std::runtime_error{error};
                }

                // NOTE it's not necessary to change the state of the spent
                // outputs because that was done when they were reserved for the
                // proposal
            }

            auto index{-1};
            auto pending = UnallocatedVector<block::Outpoint>{};
            auto tx = lmdb_.TransactionRW();

            for (const auto& output : transaction.Outputs()) {
                ++index;
                const auto keys = output.Keys();

                if (0 == keys.size()) {
                    LogTrace()(OT_PRETTY_CLASS())("output ")(
                        index)(" belongs to someone else")
                        .Flush();

                    continue;
                } else {
                    LogTrace()(OT_PRETTY_CLASS())("output ")(
                        index)(" belongs to me")
                        .Flush();
                }

                const auto& outpoint = pending.emplace_back(
                    transaction.ID().Bytes(),
                    static_cast<std::uint32_t>(index));

                if (cache.Exists(outpoint)) {
                    auto& existing = cache.GetOutput(outpoint);

                    if (false == change_state(
                                     cache,
                                     tx,
                                     outpoint,
                                     existing,
                                     node::TxoState::UnconfirmedNew,
                                     blank_)) {
                        LogError()(OT_PRETTY_CLASS())(
                            "Error updating created output state")
                            .Flush();
                        cache.Clear();

                        return false;
                    }
                } else {
                    const auto [accountID, subchainID] = [&] {
                        OT_ASSERT(1 == keys.size());

                        const auto& key = keys.at(0);
                        const auto& [nodeID, subchain, index] = key;
                        auto accountID = api_.Factory().Identifier(nodeID);
                        auto subchainID =
                            subchain_.GetSubchainID(accountID, subchain, tx);

                        return std::make_pair(
                            std::move(accountID), std::move(subchainID));
                    }();

                    if (false == create_state(
                                     cache,
                                     tx,
                                     false,
                                     outpoint,
                                     node::TxoState::UnconfirmedNew,
                                     blank_,
                                     accountID,
                                     subchainID,
                                     output)) {
                        LogError()(OT_PRETTY_CLASS())(
                            "Error creating new output state")
                            .Flush();
                        cache.Clear();

                        return false;
                    }
                }

                associate_output(outpoint, output, cache, tx);
            }

            for (const auto& outpoint : pending) {
                LogVerbose()(OT_PRETTY_CLASS())("proposal ")(proposalID.str())(
                    " created outpoint ")(outpoint.str())
                    .Flush();
                auto rc = lmdb_
                              .Store(
                                  proposal_created_,
                                  proposalID.Bytes(),
                                  outpoint.Bytes(),
                                  tx)
                              .first;

                if (false == rc) {
                    throw std::runtime_error{
                        "Failed to update proposal created outpoint index"};
                }

                rc = lmdb_
                         .Store(
                             output_proposal_,
                             outpoint.Bytes(),
                             proposalID.Bytes(),
                             tx)
                         .first;

                if (false == rc) {
                    throw std::runtime_error{
                        "Failed to update outpoint proposal index"};
                }
            }

            const auto reason = api_.Factory().PasswordPrompt(
                "Save an outgoing blockchain transaction");
            auto transactions =
                Set<std::shared_ptr<bitcoin::block::Transaction>>{
                    transaction.Internal().clone()};

            if (!api.Internal().ProcessTransactions(
                    chain_, std::move(transactions), reason)) {
                throw std::runtime_error{
                    "Error adding transaction to database"};
            }

            if (false == tx.Finalize(true)) {
                throw std::runtime_error{
                    "Failed to commit database transaction"};
            }

            // NOTE uncomment this for detailed debugging: cache.Print(lock);
            publish_balance(cache);

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            cache.Clear();

            return false;
        }
    }
    auto AdvanceTo(const block::Position& pos) noexcept -> bool
    {
        auto output{true};
        auto changed{0};
        auto handle = lock();
        auto& cache = *handle;

        try {
            const auto current = cache.GetPosition().Decode(api_);
            const auto start = current.height_;
            LogTrace()(OT_PRETTY_CLASS())("incoming position: ")(pos).Flush();
            LogTrace()(OT_PRETTY_CLASS())(" current position: ")(current)
                .Flush();

            if (pos == current) { return true; }
            if (pos.height_ < current.height_) { return true; }

            OT_ASSERT(pos.height_ > start);

            const auto stop = std::max<block::Height>(
                0,
                start - params::Chains().at(chain_).maturation_interval_ - 1);
            const auto matured = [&] {
                auto m = UnallocatedSet<block::Outpoint>{};
                lmdb_.Read(
                    generation_,
                    [&](const auto key, const auto value) {
                        const auto height = [&] {
                            auto out = block::Height{};

                            OT_ASSERT(sizeof(out) == key.size());

                            std::memcpy(&out, key.data(), key.size());

                            return out;
                        }();

                        if (is_mature(height, pos)) {
                            m.emplace(block::Outpoint{value});
                        }

                        return (height > stop);
                    },
                    Dir::Backward);

                return m;
            }();
            auto tx = lmdb_.TransactionRW();

            for (const auto& outpoint : matured) {
                static constexpr auto state = node::TxoState::ConfirmedNew;
                const auto& output = cache.GetOutput(outpoint);

                if (output.State() == state) { continue; }

                if (false == change_state(cache, tx, outpoint, state, pos)) {
                    throw std::runtime_error{"failed to mature output"};
                } else {
                    ++changed;
                }
            }

            if (false == cache.UpdatePosition(pos, tx)) {
                throw std::runtime_error{"Failed to update wallet position"};
            }

            if (false == tx.Finalize(true)) {
                throw std::runtime_error{
                    "Failed to commit database transaction"};
            }

            if (0 < changed) { publish_balance(cache); }

            return output;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            cache.Clear();

            return false;
        }
    }
    auto CancelProposal(const Identifier& id) noexcept -> bool
    {
        auto handle = lock();
        auto& cache = *handle;

        try {
            const auto reserved = [&] {
                auto out = UnallocatedVector<block::Outpoint>{};
                lmdb_.Load(
                    proposal_spent_,
                    id.Bytes(),
                    [&](const auto bytes) { out.emplace_back(bytes); },
                    Mode::Multiple);

                return out;
            }();
            const auto created = [&] {
                auto out = UnallocatedVector<block::Outpoint>{};
                lmdb_.Load(
                    proposal_created_,
                    id.Bytes(),
                    [&](const auto bytes) { out.emplace_back(bytes); },
                    Mode::Multiple);

                return out;
            }();
            auto tx = lmdb_.TransactionRW();
            auto rc{true};

            for (const auto& id : reserved) {
                rc = change_state(
                    cache,
                    tx,
                    id,
                    node::TxoState::UnconfirmedSpend,
                    node::TxoState::ConfirmedNew);

                if (false == rc) {
                    const auto error = UnallocatedCString{
                        "failed to reclaim outpoint " + id.str()};

                    throw std::runtime_error{error};
                }

                rc = lmdb_.Delete(output_proposal_, id.Bytes(), tx);

                if (false == rc) {
                    throw std::runtime_error{
                        "Failed to update outpoint - proposal index"};
                }
            }

            for (const auto& id : created) {
                auto rc = change_state(
                    cache,
                    tx,
                    id,
                    node::TxoState::UnconfirmedNew,
                    node::TxoState::OrphanedNew);

                if (false == rc) {
                    const auto error = UnallocatedCString{
                        "failed to orphan cancelled outpoint " + id.str()};

                    throw std::runtime_error{error};
                }
            }

            if (lmdb_.Exists(proposal_spent_, id.Bytes())) {
                rc = lmdb_.Delete(proposal_spent_, id.Bytes(), tx);

                if (false == rc) {
                    throw std::runtime_error{"failed to erase spent outpoints"};
                }
            } else {
                LogError()(OT_PRETTY_CLASS())("Warning: spent index for ")(
                    id.str())(" already removed")
                    .Flush();
            }

            if (lmdb_.Exists(proposal_created_, id.Bytes())) {
                rc = lmdb_.Delete(proposal_created_, id.Bytes(), tx);

                if (false == rc) {
                    throw std::runtime_error{
                        "failed to erase created outpoints"};
                }
            } else {
                LogError()(OT_PRETTY_CLASS())("Warning: created index for ")(
                    id.str())(" already removed")
                    .Flush();
            }

            rc = proposals_.CancelProposal(tx, id);

            if (false == rc) {
                throw std::runtime_error{"failed to cancel proposal"};
            }

            if (false == tx.Finalize(true)) {
                throw std::runtime_error{
                    "Failed to commit database transaction"};
            }

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            cache.Clear();

            return false;
        }
    }
    auto FinalizeReorg(MDB_txn* tx, const block::Position& pos) noexcept -> bool
    {
        auto output{true};
        auto handle = lock();
        auto& cache = *handle;

        try {
            if (cache.GetPosition().Decode(api_) != pos) {
                auto outputs = UnallocatedVector<block::Outpoint>{};
                const auto heights = [&] {
                    auto out = UnallocatedSet<block::Height>{};
                    lmdb_.Read(
                        generation_,
                        [&](const auto key, const auto value) {
                            const auto height = [&] {
                                auto out = block::Height{};

                                OT_ASSERT(sizeof(out) == key.size());

                                std::memcpy(&out, key.data(), key.size());

                                return out;
                            }();
                            outputs.emplace_back(value);

                            if (height >= pos.height_) {
                                out.emplace(height);

                                return true;
                            } else {

                                return false;
                            }
                        },
                        Dir::Backward);

                    return out;
                }();

                for (const auto height : heights) {
                    output = lmdb_.Delete(
                        generation_, static_cast<std::size_t>(height), tx);

                    if (false == output) {
                        throw std::runtime_error{
                            "failed to delete generation output index"};
                    }
                }

                for (const auto& id : outputs) {
                    using State = node::TxoState;
                    output =
                        change_state(cache, tx, id, State::OrphanedNew, pos);

                    if (false == output) {
                        throw std::runtime_error{
                            "failed to orphan generation output"};
                    }
                }

                output = cache.UpdatePosition(pos, tx);

                if (false == output) {
                    throw std::runtime_error{
                        "Failed to update wallet position"};
                }
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            output = false;
        }

        return output;
    }

    auto GetPosition() const noexcept -> block::Position
    {
        return lock_shared()->GetPosition().Decode(api_);
    }

    auto ReserveUTXO(
        const identifier::Nym& spender,
        const Identifier& id,
        node::internal::SpendPolicy& policy) noexcept -> std::optional<UTXO>
    {
        auto output = std::optional<UTXO>{std::nullopt};
        auto handle = lock();
        auto& cache = *handle;

        try {
            // TODO implement smarter selection algorithm using policy
            auto tx = lmdb_.TransactionRW();
            const auto choose =
                [&](const auto outpoint) -> std::optional<UTXO> {
                auto& existing = cache.GetOutput(outpoint);

                if (const auto& s = cache.GetNym(spender);
                    0u == s.count(outpoint)) {

                    return std::nullopt;
                }

                auto output = std::make_optional<UTXO>(
                    std::make_pair(outpoint, existing.clone()));
                auto rc = change_state(
                    cache,
                    tx,
                    outpoint,
                    existing,
                    node::TxoState::UnconfirmedSpend,
                    blank_);

                if (false == rc) {
                    throw std::runtime_error{"Failed to update outpoint state"};
                }

                rc = lmdb_
                         .Store(
                             proposal_spent_, id.Bytes(), outpoint.Bytes(), tx)
                         .first;

                if (false == rc) {
                    throw std::runtime_error{
                        "Failed to update proposal spent index"};
                }

                rc = lmdb_
                         .Store(
                             output_proposal_, outpoint.Bytes(), id.Bytes(), tx)
                         .first;

                if (false == rc) {
                    throw std::runtime_error{
                        "Failed to update outpoint proposal index"};
                }

                LogVerbose()(OT_PRETTY_CLASS())("proposal ")(id.str())(
                    " consumed outpoint ")(outpoint.str())
                    .Flush();

                return output;
            };
            const auto select = [&](const auto& group,
                                    const bool changeOnly =
                                        false) -> std::optional<UTXO> {
                for (const auto& outpoint : fifo(cache, group)) {
                    if (changeOnly) {
                        const auto& output = cache.GetOutput(outpoint);

                        if (0u == output.Tags().count(node::TxoTag::Change)) {
                            continue;
                        }
                    }

                    auto utxo = choose(outpoint);

                    if (utxo.has_value()) { return utxo; }
                }

                LogTrace()(OT_PRETTY_CLASS())(
                    "No spendable outputs for this group")
                    .Flush();

                return std::nullopt;
            };

            output = select(cache.GetState(node::TxoState::ConfirmedNew));
            const auto spendUnconfirmed =
                policy.unconfirmed_incoming_ || policy.unconfirmed_change_;

            if ((!output.has_value()) && spendUnconfirmed) {
                const auto changeOnly = !policy.unconfirmed_incoming_;
                output = select(
                    cache.GetState(node::TxoState::UnconfirmedNew), changeOnly);
            }

            if (false == output.has_value()) {
                throw std::runtime_error{
                    "No spendable outputs for specified nym"};
            }

            if (false == tx.Finalize(true)) {
                throw std::runtime_error{
                    "Failed to commit database transaction"};
            }

            return output;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            cache.Clear();

            return std::nullopt;
        }
    }
    auto StartReorg(
        MDB_txn* tx,
        const SubchainID& subchain,
        const block::Position& position) noexcept -> bool
    {
        LogTrace()(OT_PRETTY_CLASS())("rolling back block ")(position).Flush();
        auto handle = lock();
        auto& cache = *handle;
        const auto& api = api_.Crypto().Blockchain();

        try {
            // TODO rebroadcast transactions which have become unconfirmed
            const auto outpoints = [&] {
                auto out = UnallocatedSet<block::Outpoint>{};
                const auto sPosition = db::Position{position};
                lmdb_.Load(
                    positions_,
                    reader(sPosition.data_),
                    [&](const auto bytes) {
                        auto outpoint = block::Outpoint{bytes};

                        if (has_subchain(cache, subchain, outpoint)) {
                            out.emplace(std::move(outpoint));
                        }
                    },
                    Mode::Multiple);

                return out;
            }();
            LogTrace()(OT_PRETTY_CLASS())(outpoints.size())(
                " affected outpoints")
                .Flush();

            for (const auto& id : outpoints) {
                auto& output = cache.GetOutput(id);
                using State = node::TxoState;
                const auto state = [&]() -> std::optional<State> {
                    switch (output.State()) {
                        case State::ConfirmedNew:
                        case State::OrphanedNew: {

                            return State::UnconfirmedNew;
                        }
                        case State::ConfirmedSpend:
                        case State::OrphanedSpend: {

                            return State::UnconfirmedSpend;
                        }
                        default: {

                            return std::nullopt;
                        }
                    }
                }();

                if (state.has_value() &&
                    (!change_state(
                        cache, tx, id, output, state.value(), position))) {
                    throw std::runtime_error{"Failed to update output state"};
                }

                const auto& txid = api_.Factory().DataFromBytes(id.Txid());

                for (const auto& key : output.Keys()) {
                    api.Unconfirm(key, txid);
                }
            }

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            cache.Clear();

            return false;
        }
    }

    Imp(const api::Session& api,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type chain,
        const wallet::SubchainData& subchains,
        wallet::Proposal& proposals) noexcept
        : api_(api)
        , lmdb_(lmdb)
        , chain_(chain)
        , subchain_(subchains)
        , proposals_(proposals)
        , blank_(-1, block::Hash{})
        , maturation_target_(params::Chains().at(chain_).maturation_interval_)
        , cache_(api_, lmdb_, chain_, blank_)
    {
    }
    Imp() = delete;
    Imp(const Imp&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

private:
    using Cache = libguarded::shared_guarded<OutputCache, std::shared_mutex>;

    const api::Session& api_;
    const storage::lmdb::LMDB& lmdb_;
    const blockchain::Type chain_;
    const wallet::SubchainData& subchain_;
    wallet::Proposal& proposals_;
    const block::Position blank_;
    const block::Height maturation_target_;
    mutable Cache cache_;

    [[nodiscard]] static auto states(node::TxoState in) noexcept -> States
    {
        using State = node::TxoState;

        if (State::All == in) { return all_states(); }

        return States{in};
    }

    [[nodiscard]] auto effective_position(
        const node::TxoState state,
        const block::Position& oldPos,
        const block::Position& newPos) const noexcept -> const block::Position&
    {
        using State = node::TxoState;

        switch (state) {
            case State::UnconfirmedNew:
            case State::UnconfirmedSpend: {

                return oldPos;
            }
            default: {

                return newPos;
            }
        }
    }
    [[nodiscard]] auto fifo(const OutputCache& cache, const Outpoints& in)
        const noexcept -> UnallocatedVector<block::Outpoint>
    {
        auto counter = 0_uz;
        auto map =
            UnallocatedMap<block::Position, UnallocatedSet<block::Outpoint>>{};

        for (const auto& id : in) {
            const auto& output = cache.GetOutput(id);
            map[output.MinedPosition()].emplace(id);
            ++counter;
        }

        auto out = UnallocatedVector<block::Outpoint>{};
        out.reserve(counter);

        for (const auto& [position, set] : map) {
            for (const auto& id : set) { out.emplace_back(id); }
        }

        return out;
    }
    [[nodiscard]] auto get_balance(const OutputCache& cache) const noexcept
        -> Balance
    {
        static const auto blank = api_.Factory().NymID();

        return get_balance(cache, blank);
    }
    [[nodiscard]] auto get_balance(
        const OutputCache& cache,
        const identifier::Nym& owner) const noexcept -> Balance
    {
        static const auto blank = api_.Factory().Identifier();

        return get_balance(cache, owner, blank, nullptr);
    }
    [[nodiscard]] auto get_balance(
        const OutputCache& cache,
        const identifier::Nym& owner,
        const AccountID& account,
        const crypto::Key* key) const noexcept -> Balance
    {
        auto output = Balance{};
        auto& [confirmed, unconfirmed] = output;
        const auto* pNym = owner.empty() ? nullptr : &owner;
        const auto* pAcct = account.empty() ? nullptr : &account;
        auto cb = [&](const auto previous, const auto& outpoint) -> auto
        {
            const auto& existing = cache.GetOutput(outpoint);

            return previous + existing.Value();
        };

        const auto unconfirmedSpendTotal = [&] {
            const auto txos = match(
                cache,
                {node::TxoState::UnconfirmedSpend},
                pNym,
                pAcct,
                nullptr,
                key);

            return std::accumulate(txos.begin(), txos.end(), Amount{0}, cb);
        }();

        {
            const auto txos = match(
                cache,
                {node::TxoState::ConfirmedNew},
                pNym,
                pAcct,
                nullptr,
                key);
            confirmed =
                unconfirmedSpendTotal +
                std::accumulate(txos.begin(), txos.end(), Amount{0}, cb);
        }

        {
            const auto txos = match(
                cache,
                {node::TxoState::UnconfirmedNew},
                pNym,
                pAcct,
                nullptr,
                key);
            unconfirmed =
                std::accumulate(txos.begin(), txos.end(), confirmed, cb) -
                unconfirmedSpendTotal;
        }

        return output;
    }
    [[nodiscard]] auto get_balances(const OutputCache& cache) const noexcept
        -> NymBalances
    {
        auto output = NymBalances{};

        for (const auto& nym : cache.GetNyms()) {
            output[nym] = get_balance(cache, nym);
        }

        return output;
    }
    [[nodiscard]] auto get_outputs(
        const OutputCache& cache,
        const States states,
        const identifier::Nym* owner,
        const AccountID* account,
        const NodeID* subchain,
        const crypto::Key* key,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>
    {
        const auto matches =
            match(cache, states, owner, account, subchain, key);
        auto output = Vector<UTXO>{alloc};

        for (const auto& outpoint : matches) {
            const auto& existing = cache.GetOutput(outpoint);
            output.emplace_back(std::make_pair(outpoint, existing.clone()));
        }

        return output;
    }
    [[nodiscard]] auto get_unspent_outputs(
        const OutputCache& cache,
        const NodeID& id,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>
    {
        const auto* pSub = id.empty() ? nullptr : &id;

        return get_outputs(
            cache,
            {node::TxoState::UnconfirmedNew,
             node::TxoState::ConfirmedNew,
             node::TxoState::UnconfirmedSpend},
            nullptr,
            nullptr,
            pSub,
            nullptr,
            alloc);
    }
    [[nodiscard]] auto has_account(
        const OutputCache& cache,
        const AccountID& id,
        const block::Outpoint& outpoint) const noexcept -> bool
    {
        return 0 < cache.GetAccount(id).count(outpoint);
    }
    [[nodiscard]] auto has_key(
        const OutputCache& cache,
        const crypto::Key& key,
        const block::Outpoint& outpoint) const noexcept -> bool
    {
        return 0 < cache.GetKey(key).count(outpoint);
    }
    [[nodiscard]] auto has_nym(
        const OutputCache& cache,
        const identifier::Nym& id,
        const block::Outpoint& outpoint) const noexcept -> bool
    {
        return 0 < cache.GetNym(id).count(outpoint);
    }
    [[nodiscard]] auto has_subchain(
        const OutputCache& cache,
        const NodeID& id,
        const block::Outpoint& outpoint) const noexcept -> bool
    {
        return 0 < cache.GetSubchain(id).count(outpoint);
    }
    // NOTE: a mature output is available to be spent in the next block
    [[nodiscard]] auto is_mature(
        const OutputCache& cache,
        const block::Height height) const noexcept -> bool
    {
        const auto position = cache.GetPosition().Decode(api_);

        return is_mature(height, position);
    }
    [[nodiscard]] auto is_mature(
        const block::Height height,
        const block::Position& pos) const noexcept -> bool
    {
        return (pos.height_ - height) >= maturation_target_;
    }
    [[nodiscard]] auto match(
        const OutputCache& cache,
        const States states,
        const identifier::Nym* owner,
        const AccountID* account,
        const NodeID* subchain,
        const crypto::Key* key) const noexcept -> Matches
    {
        auto output = Matches{};
        const auto allSubs = (nullptr == subchain);
        const auto allAccts = (nullptr == account);
        const auto allNyms = (nullptr == owner);
        const auto allKeys = (nullptr == key);

        for (const auto state : states) {
            for (const auto& outpoint : cache.GetState(state)) {
                // NOTE if a more specific conditions is requested then
                // it's not necessary to test any more general
                // conditions. A subchain match implies an account match
                // implies a nym match
                const auto goodKey = allKeys || has_key(cache, *key, outpoint);
                const auto goodSub = (!allKeys) || allSubs ||
                                     has_subchain(cache, *subchain, outpoint);
                const auto goodAcct = (!allKeys) || (!allSubs) || allAccts ||
                                      has_account(cache, *account, outpoint);
                const auto goodNym = (!allKeys) || (!allSubs) || (!allAccts) ||
                                     allNyms ||
                                     has_nym(cache, *owner, outpoint);

                if (goodNym && goodAcct && goodSub && goodKey) {
                    output.emplace_back(outpoint);
                }
            }
        }

        return output;
    }
    auto publish_balance(const OutputCache& cache) const noexcept -> void
    {
        const auto& api = api_.Crypto().Blockchain();
        api.Internal().UpdateBalance(chain_, get_balance(cache));

        for (const auto& [nym, balance] : get_balances(cache)) {
            api.Internal().UpdateBalance(nym, chain_, balance);
        }
    }
    [[nodiscard]] auto translate(Vector<UTXO>&& outputs) const noexcept
        -> UnallocatedVector<block::pTxid>
    {
        auto out = UnallocatedVector<block::pTxid>{};
        auto temp = UnallocatedSet<block::pTxid>{};

        for (auto& [outpoint, output] : outputs) {
            temp.emplace(api_.Factory().DataFromBytes(outpoint.Txid()));
        }

        out.reserve(temp.size());
        std::move(temp.begin(), temp.end(), std::back_inserter(out));

        return out;
    }

    auto add_transactions(
        const Log& log,
        const AccountID& account,
        const SubchainID& subchain,
        const block::Position& block,
        const Parent::BlockMatches& blockMatches,
        const node::TxoState consumeState,
        const node::TxoState createState,
        Set<std::shared_ptr<bitcoin::block::Transaction>>& processed,
        TXOs& txoCreated,
        TXOs& txoConsumed,
        OutputCache& cache,
        storage::lmdb::LMDB::Transaction& tx) noexcept(false) -> void
    {
        auto consumed = Set<block::Outpoint>{txoCreated.get_allocator()};
        auto created = TXOs{txoCreated.get_allocator()};
        auto spent = TXOs{txoCreated.get_allocator()};
        auto generation = Set<block::Outpoint>{txoCreated.get_allocator()};

        for (const auto& [txid, transaction] : blockMatches) {
            const auto& [indices, pTx] = transaction;
            log(OT_PRETTY_CLASS())("parsing transaction ")(txid->asHex())
                .Flush();

            OT_ASSERT(pTx);

            processed.emplace(pTx);
            auto& modified = pTx->Internal();
            modified.SetMinedPosition(block);
            parse_inputs(block, modified, consumed, cache, tx);
            parse_outputs(indices, modified, created, generation);
        }

        // NOTE iterate over the transaction list twice to ensure that all
        // outputs created in this block are known prior to associating inputs
        // to previous outputs.

        for (const auto& [txid, transaction] : blockMatches) {
            const auto& [indices, pTx] = transaction;
            auto& modified = pTx->Internal();
            process_inputs(log, subchain, created, spent, modified, cache);
        }

        write(
            log,
            account,
            subchain,
            block,
            createState,
            generation,
            created,
            txoCreated,
            cache,
            tx);
        write(
            log,
            account,
            subchain,
            block,
            consumeState,
            generation,
            spent,
            txoConsumed,
            cache,
            tx);
    }
    auto associate_input(
        const std::size_t index,
        const bitcoin::block::internal::Output& output,
        bitcoin::block::internal::Transaction& tx) noexcept(false) -> void
    {
        if (false == tx.AssociatePreviousOutput(index, output)) {
            throw std::runtime_error{
                "error associating previous output to input"};
        }
    }
    auto associate_output(
        const block::Outpoint& outpoint,
        const bitcoin::block::Output& output,
        OutputCache& cache,
        storage::lmdb::LMDB::Transaction& tx) noexcept(false) -> void
    {
        const auto& api = api_.Crypto().Blockchain();
        const auto keys = output.Keys();

        OT_ASSERT(0 < keys.size());
        // NOTE until multisig is supported there is never a reason for
        // an output to be associated with more than one key.
        OT_ASSERT(1 == keys.size());

        for (const auto& key : keys) {
            const auto& [subaccount, subchain, bip32] = key;

            if (crypto::Subchain::Outgoing == subchain) {
                // TODO make sure this output is associated with the
                // correct contact

                continue;
            }

            const auto& owner = api.Owner(key);

            if (owner.empty()) {
                const auto error =
                    CString{"no owner found for key "}.append(print(key));

                throw std::runtime_error{error.c_str()};
            }

            // TODO AddToKey?

            if (false == cache.AddToNym(owner, outpoint, tx)) {
                throw std::runtime_error{"Error associating outpoint to nym"};
            }
        }
    }
    // Only used by CancelProposal
    [[nodiscard]] auto change_state(
        OutputCache& cache,
        MDB_txn* tx,
        const block::Outpoint& id,
        const node::TxoState oldState,
        const node::TxoState newState) noexcept -> bool
    {
        if (cache.Exists(id)) {
            auto& existing = cache.GetOutput(id);

            if (const auto state = existing.State(); state == oldState) {

                return change_state(cache, tx, id, existing, newState, blank_);
            } else if (state == newState) {
                LogVerbose()(OT_PRETTY_CLASS())("Warning: outpoint ")(id.str())(
                    " already in desired state: ")(print(newState))
                    .Flush();

                return true;
            } else {
                LogError()(OT_PRETTY_CLASS())("incorrect state for outpoint ")(
                    id.str())(". Expected: ")(print(oldState))(", actual: ")(
                    print(state))
                    .Flush();

                return false;
            }
        } else {
            LogError()(OT_PRETTY_CLASS())("outpoint ")(id.str())(
                " does not exist")
                .Flush();

            return false;
        }
    }
    [[nodiscard]] auto change_state(
        OutputCache& cache,
        MDB_txn* tx,
        const block::Outpoint& id,
        const node::TxoState newState,
        const block::Position newPosition) noexcept -> bool
    {
        if (cache.Exists(id)) {
            auto& output = cache.GetOutput(id);

            return change_state(cache, tx, id, output, newState, newPosition);
        } else {
            LogError()(OT_PRETTY_CLASS())("outpoint ")(id.str())(
                " does not exist")
                .Flush();

            return false;
        }
    }
    [[nodiscard]] auto change_state(
        OutputCache& cache,
        MDB_txn* tx,
        const block::Outpoint& id,
        bitcoin::block::internal::Output& output,
        const node::TxoState newState,
        const block::Position newPosition) noexcept -> bool
    {
        const auto oldState = output.State();
        const auto& oldPosition = output.MinedPosition();
        const auto effective =
            effective_position(newState, oldPosition, newPosition);
        const auto updateState = (newState != oldState);
        const auto updatePosition = (effective != oldPosition);
        auto rc{true};

        try {
            if (updateState) {
                rc = cache.ChangeState(oldState, newState, id, tx);

                if (false == rc) {
                    throw std::runtime_error{"Failed to update state index"};
                }

                output.SetState(newState);
            }

            if (updatePosition) {
                rc = cache.ChangePosition(oldPosition, effective, id, tx);

                if (false == rc) {
                    throw std::runtime_error{"Failed to update position index"};
                }

                output.SetMinedPosition(effective);
            }

            auto rc = cache.UpdateOutput(id, output, tx);

            if (false == rc) {
                throw std::runtime_error{"Failed to update output"};
            }

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }
    [[nodiscard]] auto check_proposals(
        OutputCache& cache,
        storage::lmdb::LMDB::Transaction& tx,
        const block::Outpoint& outpoint,
        const block::Position& block,
        const block::Txid& txid,
        UnallocatedSet<OTIdentifier>& processed) noexcept -> bool
    {
        if (-1 == block.height_) { return true; }

        auto oProposalID = std::optional<OTIdentifier>{};

        lmdb_.Load(output_proposal_, outpoint.Bytes(), [&](const auto bytes) {
            auto& id = oProposalID.emplace(api_.Factory().Identifier()).get();
            id.Assign(bytes);

            if (id.empty()) { oProposalID = std::nullopt; }
        });

        if (false == oProposalID.has_value()) { return true; }

        const auto& proposalID = oProposalID.value().get();

        if (0u < processed.count(proposalID)) { return true; }

        const auto created = [&] {
            auto out = UnallocatedVector<block::Outpoint>{};
            lmdb_.Load(
                proposal_created_,
                proposalID.Bytes(),
                [&](const auto bytes) { out.emplace_back(bytes); },
                Mode::Multiple);

            return out;
        }();
        const auto spent = [&] {
            auto out = UnallocatedVector<block::Outpoint>{};
            lmdb_.Load(
                proposal_spent_,
                proposalID.Bytes(),
                [&](const auto bytes) { out.emplace_back(bytes); },
                Mode::Multiple);

            return out;
        }();

        for (const auto& newOutpoint : created) {
            const auto rhs = api_.Factory().DataFromBytes(newOutpoint.Txid());

            if (txid != rhs) {
                static constexpr auto state = node::TxoState::OrphanedNew;
                const auto changed =
                    change_state(cache, tx, outpoint, state, block);

                if (changed) {
                    LogTrace()(OT_PRETTY_CLASS())("Updated ")(outpoint.str())(
                        " to state ")(print(state))
                        .Flush();
                } else {
                    LogError()(OT_PRETTY_CLASS())("Failed to update ")(
                        outpoint.str())(" to state ")(print(state))
                        .Flush();

                    return false;
                }
            }

            auto rc = lmdb_.Delete(
                proposal_created_, proposalID.Bytes(), newOutpoint.Bytes(), tx);

            if (rc) {
                LogTrace()(OT_PRETTY_CLASS())("Deleted index for proposal ")(
                    proposalID.str())(" to created output ")(newOutpoint.str())
                    .Flush();
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to delete index for proposal ")(proposalID.str())(
                    " to created output ")(newOutpoint.str())
                    .Flush();

                return false;
            }

            rc = lmdb_.Delete(output_proposal_, newOutpoint.Bytes(), tx);

            if (rc) {
                LogTrace()(OT_PRETTY_CLASS())(
                    "Deleted index for created outpoint ")(newOutpoint.str())(
                    " to proposal ")(proposalID.str())
                    .Flush();
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to delete index for created outpoint ")(
                    newOutpoint.str())(" to proposal ")(proposalID.str())
                    .Flush();

                return false;
            }
        }

        for (const auto& spentOutpoint : spent) {
            auto rc = lmdb_.Delete(
                proposal_spent_, proposalID.Bytes(), spentOutpoint.Bytes(), tx);

            if (rc) {
                LogTrace()(OT_PRETTY_CLASS())("Delete index for proposal ")(
                    proposalID.str())(" to consumed output ")(
                    spentOutpoint.str())
                    .Flush();
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to delete index for proposal ")(proposalID.str())(
                    " to consumed output ")(spentOutpoint.str())
                    .Flush();

                return false;
            }

            rc = lmdb_.Delete(output_proposal_, spentOutpoint.Bytes(), tx);

            if (rc) {
                LogTrace()(OT_PRETTY_CLASS())(
                    "Deleted index for consumed outpoint ")(
                    spentOutpoint.str())(" to proposal ")(proposalID.str())
                    .Flush();
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to delete index for consumed outpoint ")(
                    spentOutpoint.str())(" to proposal ")(proposalID.str())
                    .Flush();

                return false;
            }
        }

        processed.emplace(proposalID);

        return proposals_.FinishProposal(tx, proposalID);
    }
    [[nodiscard]] auto create_state(
        OutputCache& cache,
        storage::lmdb::LMDB::Transaction& tx,
        bool isGeneration,
        const block::Outpoint& id,
        const node::TxoState state,
        const block::Position position,
        const AccountID& accountID,
        const SubchainID& subchainID,
        const bitcoin::block::Output& output) noexcept -> bool
    {
        if (cache.Exists(id)) {
            cache.GetOutput(id);
            LogError()(OT_PRETTY_CLASS())("Outpoint already exists in db")
                .Flush();

            return false;
        }

        const auto& pos = effective_position(state, blank_, position);
        const auto effState = [&] {
            using State = node::TxoState;

            if (isGeneration) {
                if (State::ConfirmedNew == state) {
                    if (is_mature(cache, position.height_)) {

                        return State::ConfirmedNew;
                    } else {

                        return State::Immature;
                    }
                } else {
                    LogError()(OT_PRETTY_CLASS())(
                        "Invalid state for generation transaction output")
                        .Flush();

                    OT_FAIL;
                }
            } else {

                return state;
            }
        }();

        try {
            {
                auto pOutput = output.Internal().clone();

                {
                    OT_ASSERT(pOutput);

                    auto& created = *pOutput;

                    OT_ASSERT(0u < created.Keys().size());

                    created.SetState(effState);
                    created.SetMinedPosition(pos);

                    if (isGeneration) {
                        created.AddTag(node::TxoTag::Generation);
                    } else {
                        created.AddTag(node::TxoTag::Normal);
                    }
                }

                const auto rc = cache.AddOutput(
                    id,
                    effState,
                    pos,
                    accountID,
                    subchainID,
                    tx,
                    std::move(pOutput));

                if (false == rc) {
                    throw std::runtime_error{"failed to write output"};
                }
            }

            if (isGeneration) {
                const auto rc = lmdb_
                                    .Store(
                                        generation_,
                                        static_cast<std::size_t>(pos.height_),
                                        id.Bytes(),
                                        tx)
                                    .first;

                if (false == rc) {
                    throw std::runtime_error{
                        "failed to update generation index"};
                }
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }

        return true;
    }
    auto parse_inputs(
        const block::Position& block,
        bitcoin::block::internal::Transaction& inputTx,
        Set<block::Outpoint>& out,
        OutputCache& cache,
        storage::lmdb::LMDB::Transaction& tx) noexcept(false) -> void
    {
        auto proposals = UnallocatedSet<OTIdentifier>{};

        for (const auto& input : inputTx.Inputs()) {
            const auto& outpoint = input.PreviousOutput();

            if (false ==
                check_proposals(
                    cache, tx, outpoint, block, inputTx.ID(), proposals)) {
                throw std::runtime_error{"Error updating proposals"};
            }

            out.emplace(outpoint);
        }
    }
    auto parse_outputs(
        const Vector<std::uint32_t>& indices,
        bitcoin::block::internal::Transaction& inputTx,
        TXOs& out,
        Set<block::Outpoint>& generation) noexcept(false) -> void
    {
        for (const auto& index : indices) {
            const auto outpoint = block::Outpoint{inputTx.ID().Bytes(), index};
            const auto& output = inputTx.Outputs().at(index);

            OT_ASSERT(outpoint.Index() == index);

            if (inputTx.IsGeneration()) { generation.emplace(outpoint); }

            out.emplace(outpoint, output.Internal().clone());
        }
    }
    auto process_inputs(
        const Log& log,
        const SubchainID& subchain,
        TXOs& created,
        TXOs& spent,
        bitcoin::block::internal::Transaction& tx,
        OutputCache& cache) noexcept(false) -> void
    {
        auto index = 0_uz;

        for (const auto& input : tx.Inputs()) {
            const auto& outpoint = input.PreviousOutput();
            auto relevant{false};

            if (cache.Exists(subchain, outpoint)) {
                associate_input(index, cache.GetOutput(subchain, outpoint), tx);
                spent.emplace(outpoint, nullptr);
                relevant = true;
            } else if (auto i = created.find(outpoint); created.end() != i) {
                const auto& pOutput = i->second;

                OT_ASSERT(pOutput);

                associate_input(index, pOutput->Internal(), tx);
                spent.insert(created.extract(i));
                relevant = true;
            } else {
                // NOTE this input does not spend an output which is owned by
                // the current subchain
            }

            log(OT_PRETTY_CLASS())("input ")(index)(" of transaction ")
                .asHex(tx.ID())(" spends ")(outpoint)
                .Flush();

            if (relevant) {
                log(OT_PRETTY_CLASS())(outpoint)(" status updated to spent")
                    .Flush();
            } else {
                log(OT_PRETTY_CLASS())(outpoint)(
                    " not relevant to this subchain")
                    .Flush();
            }

            ++index;
        }
    }
    auto write(
        const Log& log,
        const AccountID& account,
        const SubchainID& subchain,
        const block::Position& block,
        const node::TxoState state,
        const Set<block::Outpoint>& generation,
        TXOs& in,
        TXOs& out,
        OutputCache& cache,
        storage::lmdb::LMDB::Transaction& tx) noexcept(false) -> void
    {
        auto stop = in.end();

        for (auto i = in.begin(); i != stop; ++i) {
            const auto& [outpoint, pOutput] = *i;

            if (cache.Exists(outpoint)) {
                auto& existing = cache.GetOutput(outpoint);

                if (change_state(cache, tx, outpoint, existing, state, block)) {
                    log(OT_PRETTY_CLASS())("output ")(outpoint)(" marked as ")(
                        print(state))
                        .Flush();
                } else {
                    throw std::runtime_error{
                        "error updating created output state"};
                }
            } else {
                OT_ASSERT(pOutput);

                const auto& output = *pOutput;
                const auto isGeneration = 0u < generation.count(outpoint);

                if (create_state(
                        cache,
                        tx,
                        isGeneration,
                        outpoint,
                        state,
                        block,
                        account,
                        subchain,
                        output)) {
                    log(OT_PRETTY_CLASS())("output ")(
                        outpoint)(" created in state ")(print(state))
                        .Flush();
                } else {
                    throw std::runtime_error{"error creating new output state"};
                }

                associate_output(outpoint, output, cache, tx);
            }
        }

        out.merge(in);
    }
};

Output::Output(
    const api::Session& api,
    const storage::lmdb::LMDB& lmdb,
    const blockchain::Type chain,
    const wallet::SubchainData& subchains,
    wallet::Proposal& proposals) noexcept
    : imp_(std::make_unique<Imp>(api, lmdb, chain, subchains, proposals))
{
    OT_ASSERT(imp_);
}

auto Output::AddConfirmedTransactions(
    const AccountID& account,
    const SubchainID& subchain,
    BatchedMatches&& transactions,
    TXOs& txoCreated,
    TXOs& txoConsumed) noexcept -> bool
{
    return imp_->AddConfirmedTransactions(
        account, subchain, std::move(transactions), txoCreated, txoConsumed);
}

auto Output::AddMempoolTransaction(
    const AccountID& account,
    const SubchainID& subchain,
    const Vector<std::uint32_t> outputIndices,
    const bitcoin::block::Transaction& transaction,
    TXOs& txoCreated) const noexcept -> bool
{
    return imp_->AddMempoolTransaction(
        account, subchain, outputIndices, transaction, txoCreated);
}

auto Output::AddOutgoingTransaction(
    const Identifier& proposalID,
    const proto::BlockchainTransactionProposal& proposal,
    const bitcoin::block::Transaction& transaction) noexcept -> bool
{
    return imp_->AddOutgoingTransaction(proposalID, proposal, transaction);
}

auto Output::AdvanceTo(const block::Position& pos) noexcept -> bool
{
    return imp_->AdvanceTo(pos);
}

auto Output::CancelProposal(const Identifier& id) noexcept -> bool
{
    return imp_->CancelProposal(id);
}

auto Output::FinalizeReorg(MDB_txn* tx, const block::Position& pos) noexcept
    -> bool
{
    return imp_->FinalizeReorg(tx, pos);
}

auto Output::GetBalance() const noexcept -> Balance
{
    return imp_->GetBalance();
}

auto Output::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    return imp_->GetBalance(owner);
}

auto Output::GetBalance(const identifier::Nym& owner, const NodeID& node)
    const noexcept -> Balance
{
    return imp_->GetBalance(owner, node);
}

auto Output::GetBalance(const crypto::Key& key) const noexcept -> Balance
{
    return imp_->GetBalance(key);
}

auto Output::GetOutputs(node::TxoState type, alloc::Resource* alloc)
    const noexcept -> Vector<UTXO>
{
    return imp_->GetOutputs(type, alloc);
}

auto Output::GetOutputs(
    const identifier::Nym& owner,
    node::TxoState type,
    alloc::Resource* alloc) const noexcept -> Vector<UTXO>
{
    return imp_->GetOutputs(owner, type, alloc);
}

auto Output::GetOutputs(
    const identifier::Nym& owner,
    const NodeID& node,
    node::TxoState type,
    alloc::Resource* alloc) const noexcept -> Vector<UTXO>
{
    return imp_->GetOutputs(owner, node, type, alloc);
}

auto Output::GetOutputs(
    const crypto::Key& key,
    node::TxoState type,
    alloc::Resource* alloc) const noexcept -> Vector<UTXO>
{
    return imp_->GetOutputs(key, type, alloc);
}

auto Output::GetOutputTags(const block::Outpoint& output) const noexcept
    -> UnallocatedSet<node::TxoTag>
{
    return imp_->GetOutputTags(output);
}

auto Output::GetPosition() const noexcept -> block::Position
{
    return imp_->GetPosition();
}

auto Output::GetTransactions() const noexcept -> UnallocatedVector<block::pTxid>
{
    return imp_->GetTransactions();
}

auto Output::GetTransactions(const identifier::Nym& account) const noexcept
    -> UnallocatedVector<block::pTxid>
{
    return imp_->GetTransactions(account);
}

auto Output::GetUnconfirmedTransactions() const noexcept
    -> UnallocatedSet<block::pTxid>
{
    return imp_->GetUnconfirmedTransactions();
}

auto Output::GetUnspentOutputs(alloc::Resource* alloc) const noexcept
    -> Vector<UTXO>
{
    return imp_->GetUnspentOutputs(alloc);
}

auto Output::GetUnspentOutputs(
    const NodeID& balanceNode,
    alloc::Resource* alloc) const noexcept -> Vector<UTXO>
{
    return imp_->GetUnspentOutputs(balanceNode, alloc);
}

auto Output::GetWalletHeight() const noexcept -> block::Height
{
    return imp_->GetWalletHeight();
}

auto Output::PublishBalance() const noexcept -> void { imp_->PublishBalance(); }

auto Output::ReserveUTXO(
    const identifier::Nym& spender,
    const Identifier& proposal,
    node::internal::SpendPolicy& policy) noexcept -> std::optional<UTXO>
{
    return imp_->ReserveUTXO(spender, proposal, policy);
}

auto Output::StartReorg(
    MDB_txn* tx,
    const SubchainID& subchain,
    const block::Position& position) noexcept -> bool
{
    return imp_->StartReorg(tx, subchain, position);
}

Output::~Output() = default;
}  // namespace opentxs::blockchain::database::wallet

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/database/wallet/Output.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <map>
#include <mutex>
#include <numeric>
#include <ostream>
#include <set>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include "Proto.hpp"
#include "Proto.tpp"
#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/Proposal.hpp"
#include "blockchain/database/wallet/Subchain.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/blockchain/node/TxoTag.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/iterator/Bidirectional.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"  // IWYU pragma: keep
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
auto key_to_bytes(const blockchain::crypto::Key& in) noexcept -> Space;
}  // namespace opentxs

namespace std
{
using Outpoint = opentxs::blockchain::block::Outpoint;
using Position = opentxs::blockchain::block::Position;
using Identifier = opentxs::OTIdentifier;
using NymID = opentxs::OTNymID;
using Key = opentxs::blockchain::crypto::Key;

template <>
struct hash<Outpoint> {
    auto operator()(const Outpoint& data) const noexcept -> std::size_t
    {
        static const auto& api = opentxs::Context().Crypto().Hash();
        static const auto key = std::array<char, 16>{};
        auto out = std::size_t{};
        const auto bytes = data.Bytes();
        api.HMAC(
            opentxs::crypto::HashType::SipHash24,
            {key.data(), key.size()},
            {std::next(bytes.data(), 24), 12u},
            opentxs::preallocated(sizeof(out), &out));

        return out;
    }
};

template <>
struct hash<Position> {
    auto operator()(const Position& data) const noexcept -> std::size_t
    {
        auto out = std::size_t{};
        const auto& hash = data.second.get();
        std::memcpy(&out, hash.data(), std::min(sizeof(out), hash.size()));

        return out;
    }
};

template <>
struct hash<Identifier> {
    auto operator()(const opentxs::Identifier& data) const noexcept
        -> std::size_t
    {
        auto out = std::size_t{};
        std::memcpy(&out, data.data(), std::min(sizeof(out), data.size()));

        return out;
    }
};

template <>
struct hash<NymID> {
    auto operator()(const NymID& data) const noexcept -> std::size_t
    {
        static const auto hasher = hash<Identifier>{};

        return hasher(data.get());
    }
};

template <>
struct hash<Key> {
    auto operator()(const Key& data) const noexcept -> std::size_t
    {
        const auto preimage = opentxs::key_to_bytes(data);
        static const auto& api = opentxs::Context().Crypto().Hash();
        static const auto key = std::array<char, 16>{};
        auto out = std::size_t{};
        api.HMAC(
            opentxs::crypto::HashType::SipHash24,
            {key.data(), key.size()},
            opentxs::reader(preimage),
            opentxs::preallocated(sizeof(out), &out));

        return out;
    }
};
}  // namespace std

namespace opentxs
{
auto key_to_bytes(const blockchain::crypto::Key& in) noexcept -> Space
{
    const auto& [id, subchain, index] = in;
    auto out = space(id.size() + sizeof(subchain) + sizeof(index));
    auto i = out.data();
    std::memcpy(i, id.data(), id.size());
    std::advance(i, id.size());
    std::memcpy(i, &subchain, sizeof(subchain));
    std::advance(i, sizeof(subchain));
    std::memcpy(i, &index, sizeof(index));
    std::advance(i, sizeof(index));

    return out;
}
}  // namespace opentxs

namespace opentxs::blockchain::database::wallet
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

using Dir = storage::lmdb::LMDB::Dir;
using Mode = storage::lmdb::LMDB::Mode;

constexpr auto accounts_{Table::AccountOutputs};
constexpr auto config_{Table::Config};
constexpr auto generation_{Table::GenerationOutputs};
constexpr auto keys_{Table::KeyOutputs};
constexpr auto nyms_{Table::NymOutputs};
constexpr auto output_proposal_{Table::OutputProposals};
constexpr auto outputs_{Table::WalletOutputs};
constexpr auto positions_{Table::PositionOutputs};
constexpr auto proposal_created_{Table::ProposalCreatedOutputs};
constexpr auto proposal_spent_{Table::ProposalSpentOutputs};
constexpr auto states_{Table::StateOutputs};
constexpr auto subchains_{Table::SubchainOutputs};

struct Output::Imp {
    auto GetBalance() const noexcept -> Balance
    {
        auto lock = sLock{lock_};

        return get_balance(lock);
    }
    auto GetBalance(const identifier::Nym& owner) const noexcept -> Balance
    {
        auto lock = sLock{lock_};

        if (owner.empty()) { return {}; }

        return get_balance(lock, owner);
    }
    auto GetBalance(const identifier::Nym& owner, const NodeID& node)
        const noexcept -> Balance
    {
        auto lock = sLock{lock_};

        if (owner.empty() || node.empty()) { return {}; }

        return get_balance(lock, owner, node, nullptr);
    }
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance
    {
        auto lock = sLock{lock_};
        static const auto owner = api_.Factory().NymID();
        static const auto node = api_.Factory().Identifier();

        return get_balance(lock, owner, node, &key);
    }
    auto GetOutputs(node::TxoState type) const noexcept -> std::vector<UTXO>
    {
        if (node::TxoState::Error == type) { return {}; }

        auto lock = sLock{lock_};

        return get_outputs(
            lock, states(type), nullptr, nullptr, nullptr, nullptr);
    }
    auto GetOutputs(const identifier::Nym& owner, node::TxoState type)
        const noexcept -> std::vector<UTXO>
    {
        if (node::TxoState::Error == type) { return {}; }

        auto lock = sLock{lock_};

        if (owner.empty()) { return {}; }

        return get_outputs(
            lock, states(type), &owner, nullptr, nullptr, nullptr);
    }
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        node::TxoState type) const noexcept -> std::vector<UTXO>
    {
        if (node::TxoState::Error == type) { return {}; }

        auto lock = sLock{lock_};

        if (owner.empty() || node.empty()) { return {}; }

        return get_outputs(lock, states(type), &owner, &node, nullptr, nullptr);
    }
    auto GetOutputs(const crypto::Key& key, node::TxoState type) const noexcept
        -> std::vector<UTXO>
    {
        if (node::TxoState::Error == type) { return {}; }

        auto lock = sLock{lock_};

        return get_outputs(lock, states(type), nullptr, nullptr, nullptr, &key);
    }
    auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> std::set<node::TxoTag>
    {
        auto lock = sLock{lock_};

        try {
            auto& existing = find_output(lock, output);

            return existing.Tags();
        } catch (...) {

            return {};
        }
    }
    auto GetTransactions() const noexcept -> std::vector<block::pTxid>
    {
        return translate(GetOutputs(node::TxoState::All));
    }
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> std::vector<block::pTxid>
    {
        return translate(GetOutputs(account, node::TxoState::All));
    }
    auto GetUnconfirmedTransactions() const noexcept -> std::set<block::pTxid>
    {
        auto out = std::set<block::pTxid>{};
        const auto unconfirmed = GetOutputs(node::TxoState::UnconfirmedNew);

        for (const auto& [outpoint, output] : unconfirmed) {
            out.emplace(api_.Factory().Data(outpoint.Txid()));
        }

        return out;
    }
    auto GetUnspentOutputs() const noexcept -> std::vector<UTXO>
    {
        static const auto blank = api_.Factory().Identifier();

        return GetUnspentOutputs(blank);
    }
    auto GetUnspentOutputs(const NodeID& id) const noexcept -> std::vector<UTXO>
    {
        auto lock = sLock{lock_};

        return get_unspent_outputs(lock, id);
    }
    auto GetWalletHeight() const noexcept -> block::Height
    {
        auto lock = sLock{lock_};

        return get_position().Height();
    }

    auto AddTransaction(
        const AccountID& account,
        const SubchainID& subchain,
        const block::Position& block,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& original,
        const node::TxoState consumed,
        const node::TxoState created) noexcept -> bool
    {
        auto lock = eLock{lock_};

        try {
            const auto isGeneration = original.IsGeneration();
            auto pCopy = original.clone();

            OT_ASSERT(pCopy);

            auto& copy = pCopy->Internal();
            copy.SetMinedPosition(block);
            auto inputIndex = std::ptrdiff_t{-1};
            auto tx = lmdb_.TransactionRW();
            auto proposals = std::set<OTIdentifier>{};

            for (const auto& input : copy.Inputs()) {
                const auto& outpoint = input.PreviousOutput();
                ++inputIndex;

                if (false ==
                    check_proposals(
                        lock, tx, outpoint, block, copy.ID(), proposals)) {
                    throw std::runtime_error{"Error updating proposals"};
                }

                try {
                    auto& existing = find_output_mutable(lock, outpoint);

                    if (!copy.AssociatePreviousOutput(
                            blockchain_, inputIndex, existing)) {
                        LogError()(OT_PRETTY_CLASS(__func__))(
                            "Error associating previous output to input")
                            .Flush();
                        clear_cache(lock);

                        return false;
                    }

                    if (false ==
                        change_state(
                            lock, tx, outpoint, existing, consumed, block)) {
                        LogError()(OT_PRETTY_CLASS(__func__))(
                            "Error updating consumed output state")
                            .Flush();
                        clear_cache(lock);

                        return false;
                    }
                } catch (...) {
                }

                // NOTE consider the case of parallel chain scanning where one
                // transaction spends inputs that belong to two different
                // subchains. The first subchain to find the transaction will
                // recognize the inputs belonging to itself but might miss the
                // inputs belonging to the other subchain if the other
                // subchain's scanning process has not yet discovered those
                // outputs. This is fine. The other scanning process will parse
                // this transaction again and at that point all inputs will be
                // recognized. The only impact is that net balance change of the
                // transaction will underestimated temporarily until scanning is
                // complete for all subchains.
            }

            for (const auto& index : outputIndices) {
                const auto outpoint = block::Outpoint{copy.ID().Bytes(), index};
                const auto& output = copy.Outputs().at(index);
                const auto keys = output.Keys();

                OT_ASSERT(0 < keys.size());
                // NOTE until multisig is supported there is never a reason for
                // an output to be associated with more than one key.
                OT_ASSERT(1 == keys.size());
                OT_ASSERT(outpoint.Index() == index);

                try {
                    auto& existing = find_output_mutable(lock, outpoint);

                    if (false ==
                        change_state(
                            lock, tx, outpoint, existing, created, block)) {
                        LogError()(OT_PRETTY_CLASS(__func__))(
                            "Error updating created output state")
                            .Flush();
                        clear_cache(lock);

                        return false;
                    }
                } catch (...) {
                    if (false == create_state(
                                     lock,
                                     tx,
                                     isGeneration,
                                     outpoint,
                                     created,
                                     block,
                                     output)) {
                        LogError()(OT_PRETTY_CLASS(__func__))(
                            "Error created new output state")
                            .Flush();
                        clear_cache(lock);

                        return false;
                    }
                }

                if (false == associate(lock, tx, outpoint, account, subchain)) {
                    throw std::runtime_error{
                        "Error associating outpoint to subchain"};
                }

                for (const auto& key : keys) {
                    const auto& [subaccount, subchain, bip32] = key;

                    if (crypto::Subchain::Outgoing == subchain) {
                        // TODO make sure this output is associated with the
                        // correct contact

                        continue;
                    }

                    const auto& owner = blockchain_.Owner(key);

                    if (owner.empty()) {
                        LogError()(OT_PRETTY_CLASS(__func__))(
                            "No owner found for key ")(opentxs::print(key))
                            .Flush();

                        OT_FAIL;
                    }

                    if (false == associate(lock, tx, outpoint, owner)) {
                        throw std::runtime_error{
                            "Error associating outpoint to nym"};
                    }
                }
            }

            const auto reason = api_.Factory().PasswordPrompt(
                "Save a received blockchain transaction");

            if (!blockchain_.Internal().ProcessTransaction(
                    chain_, copy, reason)) {
                throw std::runtime_error{
                    "Error adding transaction to database"};
            }

            if (false == tx.Finalize(true)) {
                throw std::runtime_error{
                    "Failed to commit database transaction"};
            }

            // NOTE uncomment this for detailed debugging: print(lock);
            publish_balance(lock);

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();
            clear_cache(lock);

            return false;
        }
    }
    auto AddConfirmedTransaction(
        const AccountID& account,
        const SubchainID& subchain,
        const block::Position& block,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& original) noexcept -> bool
    {
        return AddTransaction(
            account,
            subchain,
            block,
            outputIndices,
            original,
            node::TxoState::ConfirmedSpend,
            node::TxoState::ConfirmedNew);
    }
    auto AddMempoolTransaction(
        const AccountID& account,
        const SubchainID& subchain,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& original) noexcept -> bool
    {
        static const auto block = make_blank<block::Position>::value(api_);

        return AddTransaction(
            account,
            subchain,
            block,
            outputIndices,
            original,
            node::TxoState::UnconfirmedSpend,
            node::TxoState::UnconfirmedNew);
    }
    auto AddOutgoingTransaction(
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::bitcoin::Transaction& transaction) noexcept -> bool
    {
        OT_ASSERT(false == transaction.IsGeneration());

        auto lock = eLock{lock_};

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
                    const auto error = std::string{
                        "Input spending " + outpoint.str() +
                        " is not registered with a proposal"};

                    throw std::runtime_error{error};
                }

                // NOTE it's not necessary to change the state of the spent
                // outputs because that was done when they were reserved for the
                // proposal
            }

            auto index{-1};
            auto pending = std::vector<block::Outpoint>{};
            auto tx = lmdb_.TransactionRW();

            for (const auto& output : transaction.Outputs()) {
                ++index;
                const auto keys = output.Keys();

                if (0 == keys.size()) {
                    LogTrace()(OT_PRETTY_CLASS(__func__))("output ")(
                        index)(" belongs to someone else")
                        .Flush();

                    continue;
                } else {
                    LogTrace()(OT_PRETTY_CLASS(__func__))("output ")(
                        index)(" belongs to me")
                        .Flush();
                }

                OT_ASSERT(1 == keys.size());

                const auto& outpoint = pending.emplace_back(
                    transaction.ID().Bytes(),
                    static_cast<std::uint32_t>(index));

                try {
                    auto& existing = find_output_mutable(lock, outpoint);

                    if (false == change_state(
                                     lock,
                                     tx,
                                     outpoint,
                                     existing,
                                     node::TxoState::UnconfirmedNew,
                                     blank_)) {
                        LogError()(OT_PRETTY_CLASS(__func__))(
                            "Error updating created output state")
                            .Flush();
                        clear_cache(lock);

                        return false;
                    }
                } catch (...) {
                    if (false == create_state(
                                     lock,
                                     tx,
                                     false,
                                     outpoint,
                                     node::TxoState::UnconfirmedNew,
                                     blank_,
                                     output)) {
                        LogError()(OT_PRETTY_CLASS(__func__))(
                            "Error creating new output state")
                            .Flush();
                        clear_cache(lock);

                        return false;
                    }
                }

                for (const auto& key : keys) {
                    const auto& owner = blockchain_.Owner(key);

                    if (false == associate(lock, tx, outpoint, key)) {
                        throw std::runtime_error{
                            "Error associating output to subchain"};
                    }

                    if (false == associate(lock, tx, outpoint, owner)) {
                        throw std::runtime_error{
                            "Error associating output to nym"};
                    }
                }
            }

            for (const auto& outpoint : pending) {
                LogVerbose()(OT_PRETTY_CLASS(__func__))("proposal ")(
                    proposalID.str())(" created outpoint ")(outpoint.str())
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

            if (!blockchain_.Internal().ProcessTransaction(
                    chain_, transaction, reason)) {
                throw std::runtime_error{
                    "Error adding transaction to database"};
            }

            if (false == tx.Finalize(true)) {
                throw std::runtime_error{
                    "Failed to commit database transaction"};
            }

            // NOTE uncomment this for detailed debugging: print(lock);
            publish_balance(lock);

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();
            clear_cache(lock);

            return false;
        }
    }
    auto AdvanceTo(const block::Position& pos) noexcept -> bool
    {
        auto output{true};
        auto changed{0};
        auto lock = eLock{lock_};

        try {
            const auto current = get_position().Decode(api_);
            const auto start = current.first;

            if (pos == current) { return true; }
            if (pos.first < current.first) { return true; }

            OT_ASSERT(pos.first > start);

            const auto stop = std::max<block::Height>(
                0,
                start - params::Data::Chains().at(chain_).maturation_interval_ -
                    1);
            const auto matured = [&] {
                auto m = std::set<block::Outpoint>{};
                lmdb_.Read(
                    generation_,
                    [&](const auto key, const auto value) {
                        const auto height = [&] {
                            auto out = block::Height{};

                            OT_ASSERT(sizeof(out) == key.size());

                            std::memcpy(&out, key.data(), key.size());

                            return out;
                        }();

                        if (is_mature(lock, height, pos)) {
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
                const auto& output = find_output(lock, outpoint);

                if (output.State() == state) { continue; }

                if (false == change_state(lock, tx, outpoint, state, pos)) {
                    throw std::runtime_error{"failed to mature output"};
                } else {
                    ++changed;
                }
            }

            const auto sPosition = db::Position{pos};
            const auto rc = lmdb_
                                .Store(
                                    config_,
                                    tsv(database::Key::WalletPosition),
                                    reader(sPosition.data_),
                                    tx)
                                .first;

            if (false == rc) {
                throw std::runtime_error{"Failed to update wallet position"};
            }

            if (false == tx.Finalize(true)) {
                throw std::runtime_error{
                    "Failed to commit database transaction"};
            }

            cache_.position_.emplace(std::move(pos));

            if (0 < changed) { publish_balance(lock); }

            return output;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();
            clear_cache(lock);

            return false;
        }
    }
    auto CancelProposal(const Identifier& id) noexcept -> bool
    {
        auto lock = eLock{lock_};

        try {
            const auto reserved = [&] {
                auto out = std::vector<block::Outpoint>{};
                lmdb_.Load(
                    proposal_spent_,
                    id.Bytes(),
                    [&](const auto bytes) { out.emplace_back(bytes); },
                    Mode::Multiple);

                return out;
            }();
            const auto created = [&] {
                auto out = std::vector<block::Outpoint>{};
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
                    lock,
                    tx,
                    id,
                    node::TxoState::UnconfirmedSpend,
                    node::TxoState::ConfirmedNew);

                if (false == rc) {
                    const auto error =
                        std::string{"failed to reclaim outpoint " + id.str()};

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
                    lock,
                    tx,
                    id,
                    node::TxoState::UnconfirmedNew,
                    node::TxoState::OrphanedNew);

                if (false == rc) {
                    const auto error = std::string{
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
                LogError()(OT_PRETTY_CLASS(__func__))(
                    "Warning: spent index for ")(id.str())(" already removed")
                    .Flush();
            }

            if (lmdb_.Exists(proposal_created_, id.Bytes())) {
                rc = lmdb_.Delete(proposal_created_, id.Bytes(), tx);

                if (false == rc) {
                    throw std::runtime_error{
                        "failed to erase created outpoints"};
                }
            } else {
                LogError()(OT_PRETTY_CLASS(__func__))(
                    "Warning: created index for ")(id.str())(" already removed")
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
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();
            clear_cache(lock);

            return false;
        }
    }
    auto FinalizeReorg(MDB_txn* tx, const block::Position& pos) noexcept -> bool
    {
        auto output{true};
        auto lock = eLock{lock_};

        try {
            if (get_position().Decode(api_) != pos) {
                auto outputs = std::vector<block::Outpoint>{};
                const auto heights = [&] {
                    auto out = std::set<block::Height>{};
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

                            if (height >= pos.first) {
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
                        change_state(lock, tx, id, State::OrphanedNew, pos);

                    if (false == output) {
                        throw std::runtime_error{
                            "failed to orphan generation output"};
                    }
                }

                const auto sPosition = db::Position{pos};
                output = lmdb_
                             .Store(
                                 config_,
                                 tsv(database::Key::WalletPosition),
                                 reader(sPosition.data_),
                                 tx)
                             .first;

                if (false == output) {
                    throw std::runtime_error{
                        "Failed to update wallet position"};
                }

                cache_.position_.emplace(pos);
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();
            output = false;
        }

        clear_cache(lock);

        return output;
    }
    auto ReserveUTXO(
        const identifier::Nym& spender,
        const Identifier& id,
        node::internal::SpendPolicy& policy) noexcept -> std::optional<UTXO>
    {
        auto output = std::optional<UTXO>{std::nullopt};
        auto lock = eLock{lock_};

        try {
            // TODO implement smarter selection algorithm using policy
            auto tx = lmdb_.TransactionRW();
            const auto choose =
                [&](const auto outpoint) -> std::optional<UTXO> {
                auto& existing = find_output_mutable(lock, outpoint);

                if (const auto& s = find_nym(lock, spender);
                    0u == s.count(outpoint)) {

                    return std::nullopt;
                }

                auto output = std::make_optional<UTXO>(
                    std::make_pair(outpoint, existing.clone()));
                auto rc = change_state(
                    lock,
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

                LogVerbose()(OT_PRETTY_CLASS(__func__))("proposal ")(id.str())(
                    " consumed outpoint ")(outpoint.str())
                    .Flush();

                return output;
            };
            const auto select = [&](const auto& group,
                                    const bool changeOnly =
                                        false) -> std::optional<UTXO> {
                for (const auto& outpoint : fifo(lock, group)) {
                    if (changeOnly) {
                        const auto& output = find_output(lock, outpoint);

                        if (0u == output.Tags().count(node::TxoTag::Change)) {
                            continue;
                        }
                    }

                    auto utxo = choose(outpoint);

                    if (utxo.has_value()) { return utxo; }
                }

                LogTrace()(OT_PRETTY_CLASS(__func__))(
                    "No spendable outputs for this group")
                    .Flush();

                return std::nullopt;
            };

            output = select(find_state(lock, node::TxoState::ConfirmedNew));
            const auto spendUnconfirmed =
                policy.unconfirmed_incoming_ || policy.unconfirmed_change_;

            if ((!output.has_value()) && spendUnconfirmed) {
                const auto changeOnly = !policy.unconfirmed_incoming_;
                output = select(
                    find_state(lock, node::TxoState::UnconfirmedNew),
                    changeOnly);
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
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();
            clear_cache(lock);

            return std::nullopt;
        }
    }
    auto StartReorg(
        MDB_txn* tx,
        const SubchainID& subchain,
        const block::Position& position) noexcept -> bool
    {
        auto lock = eLock{lock_};

        try {
            // TODO rebroadcast transactions which have become unconfirmed
            const auto outpoints = [&] {
                auto out = std::set<block::Outpoint>{};
                const auto sPosition = db::Position{position};
                lmdb_.Load(
                    positions_,
                    reader(sPosition.data_),
                    [&](const auto bytes) {
                        auto outpoint = block::Outpoint{bytes};

                        if (has_subchain(lock, subchain, outpoint)) {
                            out.emplace(std::move(outpoint));
                        }
                    },
                    Mode::Multiple);

                return out;
            }();

            for (const auto& id : outpoints) {
                auto& output = find_output_mutable(lock, id);
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
                        lock, tx, id, output, state.value(), position))) {
                    throw std::runtime_error{"Failed to update output state"};
                }

                const auto& txid = api_.Factory().Data(id.Txid());

                for (const auto& key : output.Keys()) {
                    blockchain_.Unconfirm(key, txid);
                }
            }

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();

            return false;
        }
    }

    Imp(const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type chain,
        const wallet::SubchainData& subchains,
        wallet::Proposal& proposals) noexcept
        : api_(api)
        , blockchain_(blockchain)
        , lmdb_(lmdb)
        , chain_(chain)
        , subchain_(subchains)
        , proposals_(proposals)
        , blank_([&] {
            auto out = make_blank<block::Position>::value(api_);

            OT_ASSERT(0 < out.second->size());

            return out;
        }())
        , maturation_target_(
              params::Data::Chains().at(chain_).maturation_interval_)
        , lock_()
        , cache_()
    {
        try {
            lmdb_.Load(
                config_,
                tsv(database::Key::WalletPosition),
                [&](const auto bytes) { cache_.position_.emplace(bytes); });
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();

            OT_FAIL;
        }
    }

private:
    using SubchainID = Identifier;
    using pSubchainID = OTIdentifier;
    using States = std::vector<node::TxoState>;
    using Matches = std::vector<block::Outpoint>;
    using Outpoints = robin_hood::unordered_node_set<block::Outpoint>;
    using NymBalances = std::map<OTNymID, Balance>;
    using Nyms = robin_hood::unordered_node_set<OTNymID>;

    struct Cache {
        // NOTE if an exclusive lock is being held in the parent then locking
        // these mutexes is redundant
        std::mutex position_lock_{};
        std::optional<db::Position> position_{};
        std::mutex output_lock_{};
        robin_hood::unordered_node_map<
            block::Outpoint,
            std::unique_ptr<block::bitcoin::internal::Output>>
            outputs_{};
        std::mutex account_lock_{};
        robin_hood::unordered_node_map<OTIdentifier, Outpoints> accounts_{};
        std::mutex key_lock_{};
        robin_hood::unordered_node_map<crypto::Key, Outpoints> keys_{};
        std::mutex nym_lock_{};
        robin_hood::unordered_node_map<OTNymID, Outpoints> nyms_{};
        std::optional<Nyms> nym_list_{};
        std::mutex positions_lock_{};
        robin_hood::unordered_node_map<block::Position, Outpoints> positions_{};
        std::mutex state_lock_{};
        robin_hood::unordered_node_map<node::TxoState, Outpoints> states_{};
        std::mutex subchain_lock_{};
        robin_hood::unordered_node_map<OTIdentifier, Outpoints> subchains_{};
    };

    const api::Session& api_;
    const api::crypto::Blockchain& blockchain_;
    const storage::lmdb::LMDB& lmdb_;
    const blockchain::Type chain_;
    const wallet::SubchainData& subchain_;
    wallet::Proposal& proposals_;
    const block::Position blank_;
    const block::Height maturation_target_;
    mutable std::shared_mutex lock_;
    mutable Cache cache_;

    static auto all_states() noexcept -> const States&
    {
        using State = node::TxoState;
        static const auto data = States{
            State::UnconfirmedNew,
            State::UnconfirmedSpend,
            State::ConfirmedNew,
            State::ConfirmedSpend,
            State::OrphanedNew,
            State::OrphanedSpend,
            State::Immature,
        };

        return data;
    }
    static auto states(node::TxoState in) noexcept -> States
    {
        using State = node::TxoState;

        if (State::All == in) { return all_states(); }

        return States{in};
    }

    auto effective_position(
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
    auto fifo(const eLock& lock, const Outpoints& in) const noexcept
        -> std::vector<block::Outpoint>
    {
        auto counter = std::size_t{0};
        auto map = std::map<block::Position, std::set<block::Outpoint>>{};

        for (const auto& id : in) {
            const auto& output = find_output(lock, id);
            map[output.MinedPosition()].emplace(id);
            ++counter;
        }

        auto out = std::vector<block::Outpoint>{};
        out.reserve(counter);

        for (const auto& [position, set] : map) {
            for (const auto& id : set) { out.emplace_back(id); }
        }

        return out;
    }

    template <typename LockType>
    auto find_account(const LockType& lock, const AccountID& id) const noexcept
        -> const Outpoints&
    {
        return load_output_index(
            lock,
            accounts_,
            id,
            id.Bytes(),
            cache_.account_lock_,
            cache_.accounts_);
    }
    template <typename LockType>
    auto find_key(const LockType& lock, const crypto::Key& id) const noexcept
        -> const Outpoints&
    {
        const auto key = key_to_bytes(id);

        return load_output_index(
            lock, keys_, id, reader(key), cache_.key_lock_, cache_.keys_);
    }
    template <typename LockType>
    auto find_nym(const LockType& lock, const identifier::Nym& id)
        const noexcept -> const Outpoints&
    {
        return load_output_index(
            lock, nyms_, id, id.Bytes(), cache_.nym_lock_, cache_.nyms_);
    }
    template <typename LockType>
    auto find_output(const LockType& lock, const block::Outpoint& id) const
        noexcept(false) -> const block::bitcoin::internal::Output&
    {
        return load_output(lock, id);
    }
    template <typename LockType>
    auto find_state(const LockType& lock, node::TxoState state) const noexcept
        -> const Outpoints&
    {
        return load_output_index(
            lock,
            states_,
            state,
            static_cast<std::size_t>(state),
            cache_.state_lock_,
            cache_.states_);
    }
    template <typename LockType>
    auto find_subchain(const LockType& lock, const NodeID& id) const noexcept
        -> const Outpoints&
    {
        return load_output_index(
            lock,
            subchains_,
            id,
            id.Bytes(),
            cache_.subchain_lock_,
            cache_.subchains_);
    }
    template <typename LockType>
    auto get_balance(const LockType& lock) const noexcept -> Balance
    {
        static const auto blank = api_.Factory().NymID();

        return get_balance<LockType>(lock, blank);
    }
    template <typename LockType>
    auto get_balance(const LockType& lock, const identifier::Nym& owner)
        const noexcept -> Balance
    {
        static const auto blank = api_.Factory().Identifier();

        return get_balance<LockType>(lock, owner, blank, nullptr);
    }
    template <typename LockType>
    auto get_balance(
        const LockType& lock,
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
            const auto& existing = find_output(lock, outpoint);

            return previous + existing.Value();
        };

        const auto unconfirmedSpendTotal = [&] {
            const auto txos = match(
                lock,
                {node::TxoState::UnconfirmedSpend},
                pNym,
                pAcct,
                nullptr,
                key);

            return std::accumulate(txos.begin(), txos.end(), Amount{0}, cb);
        }();

        {
            const auto txos = match(
                lock,
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
                lock,
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
    template <typename LockType>
    auto get_balances(const LockType& lock) const noexcept -> NymBalances
    {
        auto output = NymBalances{};

        for (const auto& nym : load_nyms(lock)) {
            output[nym] = get_balance(lock, nym);
        }

        return output;
    }
    auto get_outputs(
        const sLock& lock,
        const States states,
        const identifier::Nym* owner,
        const AccountID* account,
        const NodeID* subchain,
        const crypto::Key* key) const noexcept -> std::vector<UTXO>
    {
        const auto matches = match(lock, states, owner, account, subchain, key);
        auto output = std::vector<UTXO>{};

        for (const auto& outpoint : matches) {
            const auto& existing = find_output(lock, outpoint);
            output.emplace_back(std::make_pair(outpoint, existing.clone()));
        }

        return output;
    }
    auto get_position() const noexcept -> const db::Position&
    {
        static const auto null = db::Position{blank_};
        auto lock = Lock{cache_.position_lock_};
        const auto& pos = cache_.position_;

        if (pos.has_value()) {

            return pos.value();
        } else {

            return null;
        }
    }
    auto get_unspent_outputs(const sLock& lock, const NodeID& id) const noexcept
        -> std::vector<UTXO>
    {
        const auto* pSub = id.empty() ? nullptr : &id;

        return get_outputs(
            lock,
            {node::TxoState::UnconfirmedNew,
             node::TxoState::ConfirmedNew,
             node::TxoState::UnconfirmedSpend},
            nullptr,
            nullptr,
            pSub,
            nullptr);
    }
    template <typename LockType>
    auto has_account(
        const LockType& lock,
        const AccountID& id,
        const block::Outpoint& outpoint) const noexcept -> bool
    {
        return 0 < find_account<LockType>(lock, id).count(outpoint);
    }
    template <typename LockType>
    auto has_key(
        const LockType& lock,
        const crypto::Key& key,
        const block::Outpoint& outpoint) const noexcept -> bool
    {
        return 0 < find_key<LockType>(lock, key).count(outpoint);
    }
    template <typename LockType>
    auto has_nym(
        const LockType& lock,
        const identifier::Nym& id,
        const block::Outpoint& outpoint) const noexcept -> bool
    {
        return 0 < find_nym<LockType>(lock, id).count(outpoint);
    }
    template <typename LockType>
    auto has_subchain(
        const LockType& lock,
        const NodeID& id,
        const block::Outpoint& outpoint) const noexcept -> bool
    {
        return 0 < find_subchain<LockType>(lock, id).count(outpoint);
    }
    // NOTE: a mature output is available to be spent in the next block
    template <typename LockType>
    auto is_mature(const LockType& lock, const block::Height height)
        const noexcept -> bool
    {
        const auto position = get_position().Decode(api_);

        return is_mature(lock, height, position);
    }
    template <typename LockType>
    auto is_mature(
        const LockType& lock,
        const block::Height height,
        const block::Position& pos) const noexcept -> bool
    {
        return (pos.first - height) >= maturation_target_;
    }
    template <typename LockType>
    auto load_nyms(const LockType& lock) const noexcept -> Nyms&
    {
        auto cacheLock = Lock{cache_.nym_lock_};
        auto& set = cache_.nym_list_;

        if (false == set.has_value()) {
            auto& value = set.emplace();
            auto tested = std::set<ReadView>{};
            lmdb_.Read(
                nyms_,
                [&](const auto key, const auto bytes) {
                    if (0u == tested.count(key)) {
                        value.emplace([&] {
                            auto out = api_.Factory().NymID();
                            out->Assign(key);

                            return out;
                        }());
                        tested.emplace(key);
                    }

                    return true;
                },
                Dir::Forward);
        }

        return set.value();
    }
    template <typename LockType>
    auto load_output(const LockType& lock, const block::Outpoint& id) const
        noexcept(false) -> block::bitcoin::internal::Output&
    {
        auto cacheLock = Lock{cache_.output_lock_};
        auto& map = cache_.outputs_;
        auto it = map.find(id);

        if (map.end() != it) {
            auto& out = *it->second;

            OT_ASSERT(0 < out.Keys().size());

            return out;
        }

        lmdb_.Load(outputs_, id.Bytes(), [&](const auto bytes) {
            const auto proto =
                proto::Factory<proto::BlockchainTransactionOutput>(bytes);
            auto pOutput = factory::BitcoinTransactionOutput(
                api_, blockchain_, chain_, proto);

            if (pOutput) {
                auto [row, added] = map.try_emplace(id, std::move(pOutput));

                OT_ASSERT(added);

                it = row;
            }
        });

        if (map.end() != it) {
            auto& out = *it->second;

            OT_ASSERT(0 < out.Keys().size());

            return out;
        }

        const auto error = std::string{"output "} + id.str() + " not found";

        throw std::out_of_range{error};
    }
    template <
        typename LockType,
        typename MapKeyType,
        typename DBKeyType,
        typename MapType>
    auto load_output_index(
        const LockType& lock,
        const Table table,
        const MapKeyType& key,
        const DBKeyType dbKey,
        std::mutex& cacheMutex,
        MapType& map) const noexcept -> const Outpoints&
    {
        static const auto empty = Outpoints{};
        auto cacheLock = Lock{cacheMutex};
        auto it = map.find(key);

        if (map.end() != it) { return it->second; }

        auto [row, added] = map.try_emplace(key, Outpoints{});

        OT_ASSERT(added);

        auto& set = row->second;
        const auto loaded = lmdb_.Load(
            table,
            dbKey,
            [&](const auto bytes) { set.emplace(bytes); },
            Mode::Multiple);

        if (loaded) {

            return row->second;
        } else {
            map.erase(row);

            return empty;
        }
    }
    template <typename LockType>
    auto match(
        const LockType& lock,
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
            for (const auto& outpoint : find_state(lock, state)) {
                // NOTE if a more specific conditions is requested then it's
                // not necessary to test any more general conditions. A subchain
                // match implies an account match implies a nym match
                const auto goodKey = allKeys || has_key(lock, *key, outpoint);
                const auto goodSub = (!allKeys) || allSubs ||
                                     has_subchain(lock, *subchain, outpoint);
                const auto goodAcct = (!allKeys) || (!allSubs) || allAccts ||
                                      has_account(lock, *account, outpoint);
                const auto goodNym = (!allKeys) || (!allSubs) || (!allAccts) ||
                                     allNyms || has_nym(lock, *owner, outpoint);

                if (goodNym && goodAcct && goodSub && goodKey) {
                    output.emplace_back(outpoint);
                }
            }
        }

        return output;
    }
    // TODO read from database instead of cache
    auto print(const eLock&) const noexcept -> void
    {
        struct Output {
            std::stringstream text_{};
            Amount total_{};
        };
        auto output = std::map<node::TxoState, Output>{};

        for (const auto& data : cache_.outputs_) {
            const auto& outpoint = data.first;
            const auto& item = *data.second;
            auto& out = output[item.State()];
            out.text_ << "\n * " << outpoint.str() << ' ';
            out.text_ << " value: " << item.Value().str();
            out.total_ += item.Value();
            const auto& script = item.Script();
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

            out.text_ << ", state: " << opentxs::print(item.State());
        }

        const auto& unconfirmed = output[node::TxoState::UnconfirmedNew];
        const auto& confirmed = output[node::TxoState::ConfirmedNew];
        const auto& pending = output[node::TxoState::UnconfirmedSpend];
        const auto& spent = output[node::TxoState::ConfirmedSpend];
        const auto& orphan = output[node::TxoState::OrphanedNew];
        const auto& outgoingOrphan = output[node::TxoState::OrphanedSpend];
        const auto& immature = output[node::TxoState::Immature];
        LogError()(OT_PRETTY_CLASS(__func__))("Instance ")(api_.Instance())(
            " TXO database contents:")
            .Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Unconfirmed available value: ")(
            unconfirmed.total_)(unconfirmed.text_.str())
            .Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Confirmed available value: ")(
            confirmed.total_)(confirmed.text_.str())
            .Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Unconfirmed spent value: ")(
            pending.total_)(pending.text_.str())
            .Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Confirmed spent value: ")(
            spent.total_)(spent.text_.str())
            .Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Orphaned incoming value: ")(
            orphan.total_)(orphan.text_.str())
            .Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Orphaned spend value: ")(
            outgoingOrphan.total_)(outgoingOrphan.text_.str())
            .Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Immature value: ")(
            immature.total_)(immature.text_.str())
            .Flush();

        LogError()(OT_PRETTY_CLASS(__func__))("Outputs by block:\n");

        for (const auto& [position, outputs] : cache_.positions_) {
            const auto& [height, hash] = position;
            LogError()("  * block ")(hash->asHex())(" at height ")(
                height)("\n");

            for (const auto& outpoint : outputs) {
                LogError()("    * ")(outpoint.str())("\n");
            }
        }

        LogError().Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Outputs by nym:\n");

        for (const auto& [id, outputs] : cache_.nyms_) {
            LogError()("  * ")(id->str())("\n");

            for (const auto& outpoint : outputs) {
                LogError()("    * ")(outpoint.str())("\n");
            }
        }

        LogError().Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Outputs by subaccount:\n");

        for (const auto& [id, outputs] : cache_.accounts_) {
            LogError()("  * ")(id->str())("\n");

            for (const auto& outpoint : outputs) {
                LogError()("    * ")(outpoint.str())("\n");
            }
        }

        LogError().Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Outputs by subchain:\n");

        for (const auto& [id, outputs] : cache_.subchains_) {
            LogError()("  * ")(id->str())("\n");

            for (const auto& outpoint : outputs) {
                LogError()("    * ")(outpoint.str())("\n");
            }
        }

        LogError().Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Outputs by key:\n");

        for (const auto& [key, outputs] : cache_.keys_) {
            LogError()("  * ")(opentxs::print(key))("\n");

            for (const auto& outpoint : outputs) {
                LogError()("    * ")(outpoint.str())("\n");
            }
        }

        LogError().Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Outputs by state:\n");

        for (const auto& [state, outputs] : cache_.states_) {
            LogError()("  * ")(opentxs::print(state))("\n");

            for (const auto& outpoint : outputs) {
                LogError()("    * ")(outpoint.str())("\n");
            }
        }

        LogError().Flush();
        LogError()(OT_PRETTY_CLASS(__func__))("Generation outputs:\n");
        lmdb_.Read(
            generation_,
            [](const auto key, const auto value) -> bool {
                const auto height = [&] {
                    auto out = block::Height{};

                    OT_ASSERT(sizeof(out) == key.size());

                    std::memcpy(&out, key.data(), key.size());

                    return out;
                }();
                const auto outpoint = block::Outpoint{value};
                LogError()("  * height ")(height)(", ")(outpoint.str())("\n");

                return true;
            },
            Dir::Forward);

        LogError().Flush();
    }
    auto publish_balance(const eLock& lock) const noexcept -> void
    {
        blockchain_.Internal().UpdateBalance(chain_, get_balance(lock));

        for (const auto& [nym, balance] : get_balances(lock)) {
            blockchain_.Internal().UpdateBalance(nym, chain_, balance);
        }
    }
    auto translate(std::vector<UTXO>&& outputs) const noexcept
        -> std::vector<block::pTxid>
    {
        auto out = std::vector<block::pTxid>{};
        auto temp = std::set<block::pTxid>{};

        for (auto& [outpoint, output] : outputs) {
            temp.emplace(api_.Factory().Data(outpoint.Txid()));
        }

        out.reserve(temp.size());
        std::move(temp.begin(), temp.end(), std::back_inserter(out));

        return out;
    }

    auto associate(
        const eLock& lock,
        storage::lmdb::LMDB::Transaction& tx,
        const block::Outpoint& outpoint,
        const crypto::Key& key) noexcept -> bool
    {
        const auto& [nodeID, subchain, index] = key;
        const auto accountID = api_.Factory().Identifier(nodeID);
        const auto subchainID =
            subchain_.GetSubchainID(accountID, subchain, tx);

        return associate(lock, tx, outpoint, accountID, subchainID);
    }
    auto associate(
        const eLock& lock,
        storage::lmdb::LMDB::Transaction& tx,
        const block::Outpoint& outpoint,
        const AccountID& accountID,
        const SubchainID& subchainID) noexcept -> bool
    {
        OT_ASSERT(false == accountID.empty());
        OT_ASSERT(false == subchainID.empty());

        try {
            auto rc =
                lmdb_.Store(accounts_, accountID.Bytes(), outpoint.Bytes(), tx)
                    .first;

            if (false == rc) {
                throw std::runtime_error{"Failed to update account index"};
            }

            rc =
                lmdb_
                    .Store(subchains_, subchainID.Bytes(), outpoint.Bytes(), tx)
                    .first;

            if (false == rc) {
                throw std::runtime_error{"Failed to update subchain index"};
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();

            return false;
        }

        cache_.accounts_[accountID].emplace(outpoint);
        cache_.subchains_[subchainID].emplace(outpoint);

        return true;
    }
    auto associate(
        const eLock& lock,
        storage::lmdb::LMDB::Transaction& tx,
        const block::Outpoint& outpoint,
        const identifier::Nym& nymID) noexcept -> bool
    {
        OT_ASSERT(false == nymID.empty());

        try {
            auto rc =
                lmdb_.Store(nyms_, nymID.Bytes(), outpoint.Bytes(), tx).first;

            if (false == rc) {
                throw std::runtime_error{"Failed to update nym index"};
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();

            return false;
        }

        cache_.nyms_[OTNymID{nymID}].emplace(outpoint);
        load_nyms(lock).emplace(nymID);

        return true;
    }
    // Only used by CancelProposal
    auto change_state(
        const eLock& lock,
        MDB_txn* tx,
        const block::Outpoint& id,
        const node::TxoState oldState,
        const node::TxoState newState) noexcept -> bool
    {
        try {
            auto& existing = find_output_mutable(lock, id);

            if (const auto state = existing.State(); state == oldState) {

                return change_state(lock, tx, id, existing, newState, blank_);
            } else if (state == newState) {
                LogVerbose()(OT_PRETTY_CLASS(__func__))("Warning: outpoint ")(
                    id.str())(" already in desired state: ")(
                    opentxs::print(newState))
                    .Flush();

                return true;
            } else {
                LogError()(OT_PRETTY_CLASS(__func__))(
                    "incorrect state for outpoint ")(id.str())(". Expected: ")(
                    opentxs::print(oldState))(", actual: ")(
                    opentxs::print(state))
                    .Flush();

                return false;
            }
        } catch (...) {
            LogError()(OT_PRETTY_CLASS(__func__))("outpoint ")(id.str())(
                " does not exist")
                .Flush();

            return false;
        }
    }
    auto change_state(
        const eLock& lock,
        MDB_txn* tx,
        const block::Outpoint& id,
        const node::TxoState newState,
        const block::Position newPosition) noexcept -> bool
    {
        try {
            auto& output = find_output_mutable(lock, id);

            return change_state(lock, tx, id, output, newState, newPosition);
        } catch (...) {
            LogError()(OT_PRETTY_CLASS(__func__))("outpoint ")(id.str())(
                " does not exist")
                .Flush();

            return false;
        }
    }
    auto change_state(
        const eLock& lock,
        MDB_txn* tx,
        const block::Outpoint& id,
        block::bitcoin::internal::Output& output,
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
                auto deleted = std::vector<node::TxoState>{};

                for (const auto state : all_states()) {
                    if (lmdb_.Delete(
                            states_,
                            static_cast<std::size_t>(state),
                            id.Bytes(),
                            tx)) {
                        deleted.emplace_back(state);
                    }
                }

                if ((0u == deleted.size()) || (oldState != deleted.front())) {
                    LogError()(OT_PRETTY_CLASS(__func__))(
                        "Warning: state index for ")(id.str())(
                        " did not match expected value")
                        .Flush();
                }

                rc = lmdb_
                         .Store(
                             states_,
                             static_cast<std::size_t>(newState),
                             id.Bytes(),
                             tx)
                         .first;

                if (false == rc) {
                    throw std::runtime_error{"Failed to add new state index"};
                }
            }

            if (updatePosition) {
                const auto oldP = db::Position{oldPosition};
                const auto newP = db::Position{newPosition};

                if (lmdb_.Exists(positions_, reader(oldP.data_), id.Bytes())) {
                    rc = lmdb_.Delete(
                        positions_, reader(oldP.data_), id.Bytes(), tx);

                    if (false == rc) {
                        throw std::runtime_error{
                            "Failed to remove old position index"};
                    }
                } else {
                    LogError()(OT_PRETTY_CLASS(__func__))(
                        "Warning: position index for ")(id.str())(
                        " already removed")
                        .Flush();
                }

                rc = lmdb_.Store(positions_, reader(newP.data_), id.Bytes(), tx)
                         .first;

                if (false == rc) {
                    throw std::runtime_error{
                        "Failed to add position state index"};
                }
            }

            if (updateState) {
                auto& from = cache_.states_[oldState];
                auto& to = cache_.states_[newState];
                from.erase(id);
                to.emplace(id);
                output.SetState(newState);
            }

            if (updatePosition) {
                auto& from = cache_.positions_[oldPosition];
                auto& to = cache_.positions_[effective];
                from.erase(id);
                to.emplace(id);
                output.SetMinedPosition(effective);
            }

            auto rc = write_output(lock, tx, id, output);

            if (false == rc) {
                throw std::runtime_error{"Failed to update output"};
            }

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();

            return false;
        }
    }
    auto check_proposals(
        const eLock& lock,
        storage::lmdb::LMDB::Transaction& tx,
        const block::Outpoint& outpoint,
        const block::Position& block,
        const block::Txid& txid,
        std::set<OTIdentifier>& processed) noexcept -> bool
    {
        if (-1 == block.first) { return true; }

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
            auto out = std::vector<block::Outpoint>{};
            lmdb_.Load(
                proposal_created_,
                proposalID.Bytes(),
                [&](const auto bytes) { out.emplace_back(bytes); },
                Mode::Multiple);

            return out;
        }();
        const auto spent = [&] {
            auto out = std::vector<block::Outpoint>{};
            lmdb_.Load(
                proposal_spent_,
                proposalID.Bytes(),
                [&](const auto bytes) { out.emplace_back(bytes); },
                Mode::Multiple);

            return out;
        }();

        for (const auto& newOutpoint : created) {
            const auto rhs = api_.Factory().Data(newOutpoint.Txid());

            if (txid != rhs) {
                static constexpr auto state = node::TxoState::OrphanedNew;
                const auto changed =
                    change_state(lock, tx, outpoint, state, block);

                if (changed) {
                    LogTrace()(OT_PRETTY_CLASS(__func__))("Updated ")(
                        outpoint.str())(" to state ")(opentxs::print(state))
                        .Flush();
                } else {
                    LogError()(OT_PRETTY_CLASS(__func__))("Failed to update ")(
                        outpoint.str())(" to state ")(opentxs::print(state))
                        .Flush();

                    return false;
                }
            }

            auto rc = lmdb_.Delete(
                proposal_created_, proposalID.Bytes(), newOutpoint.Bytes(), tx);

            if (rc) {
                LogTrace()(OT_PRETTY_CLASS(__func__))(
                    "Deleted index for proposal ")(proposalID.str())(
                    " to created output ")(newOutpoint.str())
                    .Flush();
            } else {
                LogError()(OT_PRETTY_CLASS(__func__))(
                    "Failed to delete index for proposal ")(proposalID.str())(
                    " to created output ")(newOutpoint.str())
                    .Flush();

                return false;
            }

            rc = lmdb_.Delete(output_proposal_, newOutpoint.Bytes(), tx);

            if (rc) {
                LogTrace()(OT_PRETTY_CLASS(__func__))(
                    "Deleted index for created outpoint ")(newOutpoint.str())(
                    " to proposal ")(proposalID.str())
                    .Flush();
            } else {
                LogError()(OT_PRETTY_CLASS(__func__))(
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
                LogTrace()(OT_PRETTY_CLASS(__func__))(
                    "Delete index for proposal ")(proposalID.str())(
                    " to consumed output ")(spentOutpoint.str())
                    .Flush();
            } else {
                LogError()(OT_PRETTY_CLASS(__func__))(
                    "Failed to delete index for proposal ")(proposalID.str())(
                    " to consumed output ")(spentOutpoint.str())
                    .Flush();

                return false;
            }

            rc = lmdb_.Delete(output_proposal_, spentOutpoint.Bytes(), tx);

            if (rc) {
                LogTrace()(OT_PRETTY_CLASS(__func__))(
                    "Deleted index for consumed outpoint ")(
                    spentOutpoint.str())(" to proposal ")(proposalID.str())
                    .Flush();
            } else {
                LogError()(OT_PRETTY_CLASS(__func__))(
                    "Failed to delete index for consumed outpoint ")(
                    spentOutpoint.str())(" to proposal ")(proposalID.str())
                    .Flush();

                return false;
            }
        }

        processed.emplace(proposalID);

        return proposals_.FinishProposal(tx, proposalID);
    }
    auto clear_cache(const eLock&) noexcept -> void
    {
        cache_.position_ = std::nullopt;
        cache_.nym_list_ = std::nullopt;
        cache_.outputs_.clear();
        cache_.accounts_.clear();
        cache_.keys_.clear();
        cache_.nyms_.clear();
        cache_.positions_.clear();
        cache_.states_.clear();
        cache_.subchains_.clear();
    }
    auto create_state(
        const eLock& lock,
        storage::lmdb::LMDB::Transaction& tx,
        bool isGeneration,
        const block::Outpoint& id,
        const node::TxoState state,
        const block::Position position,
        const block::bitcoin::Output& output) noexcept -> bool
    {
        try {
            [[maybe_unused]] const auto& existing = find_output(lock, id);
            LogError()(OT_PRETTY_CLASS(__func__))(
                "Outpoint already exists in db")
                .Flush();

            return false;
        } catch (...) {
        }

        const auto& pos = effective_position(state, blank_, position);
        const auto effState = [&] {
            using State = node::TxoState;

            if (isGeneration) {
                if (State::ConfirmedNew == state) {
                    if (is_mature(lock, position.first)) {

                        return State::ConfirmedNew;
                    } else {

                        return State::Immature;
                    }
                } else {
                    LogError()(OT_PRETTY_CLASS(__func__))(
                        "Invalid state for generation transaction output")
                        .Flush();

                    OT_FAIL;
                }
            } else {

                return state;
            }
        }();
        auto pOutput = output.Internal().clone();

        OT_ASSERT(pOutput);

        try {
            auto& created = *pOutput;

            OT_ASSERT(0u < created.Keys().size());

            created.SetState(effState);
            created.SetMinedPosition(pos);

            if (isGeneration) {
                created.AddTag(node::TxoTag::Generation);
            } else {
                created.AddTag(node::TxoTag::Normal);
            }

            const auto sPos = db::Position{pos};
            auto rc = write_output(lock, tx, id, created);

            if (false == rc) {
                throw std::runtime_error{"failed to write output"};
            }

            rc = lmdb_
                     .Store(
                         states_,
                         static_cast<std::size_t>(effState),
                         id.Bytes(),
                         tx)
                     .first;

            if (false == rc) {
                throw std::runtime_error{"update to state index"};
            }

            rc = lmdb_.Store(positions_, reader(sPos.data_), id.Bytes(), tx)
                     .first;

            if (false == rc) {
                throw std::runtime_error{"update to position index"};
            }

            for (const auto& key : created.Keys()) {
                const auto sKey = key_to_bytes(key);
                rc = lmdb_.Store(keys_, reader(sKey), id.Bytes(), tx).first;

                if (false == rc) {
                    throw std::runtime_error{"update to key index"};
                }
            }

            if (isGeneration) {
                rc = lmdb_
                         .Store(
                             generation_,
                             static_cast<std::size_t>(pos.first),
                             id.Bytes(),
                             tx)
                         .first;

                if (false == rc) {
                    throw std::runtime_error{"update to generation index"};
                }
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS(__func__))(e.what()).Flush();

            return false;
        }

        cache_.outputs_.try_emplace(id, std::move(pOutput));
        cache_.states_[effState].emplace(id);
        cache_.positions_[pos].emplace(id);

        for (const auto& key : output.Keys()) { cache_.keys_[key].emplace(id); }

        return true;
    }
    auto find_output_mutable(
        const eLock& lock,
        const block::Outpoint& id) noexcept(false)
        -> block::bitcoin::internal::Output&
    {
        return load_output(lock, id);
    }
    auto write_output(
        const eLock& lock,
        MDB_txn* tx,
        const block::Outpoint& id,
        const block::bitcoin::internal::Output& output) noexcept -> bool
    {
        const auto serialized = [&] {
            auto out = Space{};
            const auto data = [&] {
                auto proto = block::bitcoin::internal::Output::SerializeType{};
                const auto rc = output.Serialize(blockchain_, proto);

                if (false == rc) {
                    throw std::runtime_error{"failed to serialize as protobuf"};
                }

                return proto;
            }();

            if (false == proto::write(data, writer(out))) {
                throw std::runtime_error{"failed to serialize as bytes"};
            }

            return out;
        }();

        return lmdb_.Store(outputs_, id.Bytes(), reader(serialized), tx).first;
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Output::Output(
    const api::Session& api,
    const api::crypto::Blockchain& blockchain,
    const storage::lmdb::LMDB& lmdb,
    const blockchain::Type chain,
    const wallet::SubchainData& subchains,
    wallet::Proposal& proposals) noexcept
    : imp_(std::make_unique<
           Imp>(api, blockchain, lmdb, chain, subchains, proposals))
{
    OT_ASSERT(imp_);
}

auto Output::AddConfirmedTransaction(
    const AccountID& account,
    const SubchainID& subchain,
    const block::Position& block,
    const std::size_t blockIndex,
    const std::vector<std::uint32_t> outputIndices,
    const block::bitcoin::Transaction& transaction) noexcept -> bool
{
    return imp_->AddConfirmedTransaction(
        account, subchain, block, outputIndices, transaction);
}

auto Output::AddMempoolTransaction(
    const AccountID& account,
    const SubchainID& subchain,
    const std::vector<std::uint32_t> outputIndices,
    const block::bitcoin::Transaction& transaction) const noexcept -> bool
{
    return imp_->AddMempoolTransaction(
        account, subchain, outputIndices, transaction);
}

auto Output::AddOutgoingTransaction(
    const Identifier& proposalID,
    const proto::BlockchainTransactionProposal& proposal,
    const block::bitcoin::Transaction& transaction) noexcept -> bool
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

auto Output::GetOutputs(node::TxoState type) const noexcept -> std::vector<UTXO>
{
    return imp_->GetOutputs(type);
}

auto Output::GetOutputs(const identifier::Nym& owner, node::TxoState type)
    const noexcept -> std::vector<UTXO>
{
    return imp_->GetOutputs(owner, type);
}

auto Output::GetOutputs(
    const identifier::Nym& owner,
    const NodeID& node,
    node::TxoState type) const noexcept -> std::vector<UTXO>
{
    return imp_->GetOutputs(owner, node, type);
}

auto Output::GetOutputs(const crypto::Key& key, node::TxoState type)
    const noexcept -> std::vector<UTXO>
{
    return imp_->GetOutputs(key, type);
}

auto Output::GetOutputTags(const block::Outpoint& output) const noexcept
    -> std::set<node::TxoTag>
{
    return imp_->GetOutputTags(output);
}

auto Output::GetTransactions() const noexcept -> std::vector<block::pTxid>
{
    return imp_->GetTransactions();
}

auto Output::GetTransactions(const identifier::Nym& account) const noexcept
    -> std::vector<block::pTxid>
{
    return imp_->GetTransactions(account);
}

auto Output::GetUnconfirmedTransactions() const noexcept
    -> std::set<block::pTxid>
{
    return imp_->GetUnconfirmedTransactions();
}

auto Output::GetUnspentOutputs() const noexcept -> std::vector<UTXO>
{
    return imp_->GetUnspentOutputs();
}

auto Output::GetUnspentOutputs(const NodeID& balanceNode) const noexcept
    -> std::vector<UTXO>
{
    return imp_->GetUnspentOutputs(balanceNode);
}

auto Output::GetWalletHeight() const noexcept -> block::Height
{
    return imp_->GetWalletHeight();
}

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

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/client/wallet/Wallet.hpp"  // IWYU pragma: associated

#include <chrono>
#include <optional>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/blockchain/PaymentCode.hpp"
#include "opentxs/api/client/blockchain/Subchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/BlockchainTransaction.pb.h"
#include "opentxs/protobuf/BlockchainTransactionProposal.pb.h"
#include "opentxs/protobuf/BlockchainTransactionProposedNotification.pb.h"
#include "opentxs/protobuf/BlockchainTransactionProposedOutput.pb.h"
#include "util/ScopeGuard.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::client::implementation::Wallet::Proposals::"

namespace opentxs::blockchain::client::implementation
{
Wallet::Proposals::Proposals(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const internal::Network& network,
    const internal::WalletDatabase& db,
    const Type chain) noexcept
    : api_(api)
    , blockchain_(blockchain)
    , network_(network)
    , db_(db)
    , chain_(chain)
    , lock_()
    , pending_()
    , confirming_()
{
    for (const auto& serialized : db_.LoadProposals()) {
        const auto id = api_.Factory().Identifier(serialized.id());

        if (serialized.has_finished()) {
            confirming_.emplace(std::move(id), Time{});
        } else {
            pending_[id];
        }
    }
}

auto Wallet::Proposals::Add(const Proposal& tx) const noexcept
    -> std::future<block::pTxid>
{
    const auto id = api_.Factory().Identifier(tx.id());
    Lock lock{lock_};

    if (0 < pending_.count(id)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Proposal already exists").Flush();

        return {};
    }

    if (false == db_.AddProposal(id, tx)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Database error").Flush();

        return {};
    }

    return pending_[id].get_future();
}

auto Wallet::Proposals::build_transaction_bitcoin(
    const Identifier& id,
    Proposal& proposal,
    std::promise<block::pTxid>& promise) const noexcept -> BuildResult
{
    auto output = BuildResult::Success;
    auto builder = BitcoinTransactionBuilder{
        api_, blockchain_, db_, id, chain_, network_.FeeRate()};

    for (const auto& output : proposal.output()) {
        if (false == output.has_paymentcodechannel()) { continue; }

        using Subchain = api::client::blockchain::Subchain;
        builder.keys_to_reclaim_.emplace(
            output.paymentcodechannel(), Subchain::Outgoing, output.index());
    }

    auto post = ScopeGuard{[&] {
        if (BuildResult::PermanentFailure != output) { return; }

        for (const auto& key : builder.keys_to_reclaim_) {
            blockchain_.Release(key);
        }
    }};

    if (false == builder.CreateOutputs(proposal)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create outputs")
            .Flush();
        output = BuildResult::PermanentFailure;

        return output;
    }

    if (false == builder.AddChange(proposal)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate change output")
            .Flush();
        output = BuildResult::PermanentFailure;

        return output;
    }

    while (false == builder.IsFunded()) {
        auto utxo = db_.ReserveUTXO(id);

        if (false == utxo.has_value()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Insufficient funds").Flush();
            output = BuildResult::PermanentFailure;

            return output;
        }

        if (false == builder.AddInput(utxo.value())) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to add input").Flush();
            output = BuildResult::PermanentFailure;

            return output;
        }
    }

    OT_ASSERT(builder.IsFunded());

    builder.FinalizeOutputs();

    if (false == builder.SignInputs()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign inputs").Flush();
        output = BuildResult::PermanentFailure;

        return output;
    }

    auto pTransaction = builder.FinalizeTransaction();

    if (false == bool(pTransaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate transaction")
            .Flush();
        output = BuildResult::PermanentFailure;

        return output;
    }

    const auto& transaction = *pTransaction;
    auto proto = transaction.Serialize(blockchain_);

    if (false == proto.has_value()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize transaction")
            .Flush();
        output = BuildResult::PermanentFailure;

        return output;
    }

    *proposal.mutable_finished() = proto.value();

    if (!db_.AddProposal(id, proposal)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Database error (proposal)")
            .Flush();
        output = BuildResult::PermanentFailure;

        return output;
    }

    if (!db_.AddOutgoingTransaction(chain_, id, proposal, transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Database error (transaction)")
            .Flush();
        output = BuildResult::PermanentFailure;

        return output;
    }

    promise.set_value(transaction.ID());
    const auto sent = network_.BroadcastTransaction(transaction);

    try {
        if (false == sent) { throw std::runtime_error{"Failed to send tx"}; }

        for (const auto& notif : proposal.notification()) {
            using PC = api::client::blockchain::internal::PaymentCode;
            const auto accountID = PC::GetID(
                api_,
                chain_,
                api_.Factory().PaymentCode(notif.sender()),
                api_.Factory().PaymentCode(notif.recipient()));
            const auto& account = blockchain_.PaymentCodeSubaccount(
                api_.Factory().NymID(proposal.initiator()), accountID);
            account.AddNotification(transaction.ID());
        }
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
    }

    return output;
}

auto Wallet::Proposals::cleanup(const Lock& lock) noexcept -> void
{
    const auto finished = db_.CompletedProposals();

    for (const auto& id : finished) {
        pending_.erase(id);
        confirming_.erase(id);
    }

    db_.ForgetProposals(finished);
}

auto Wallet::Proposals::get_builder() const noexcept -> Builder
{
    switch (chain_) {
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest: {

            return [this](
                       const Identifier& id,
                       Proposal& in,
                       std::promise<block::pTxid>& out) -> auto
            {
                return build_transaction_bitcoin(id, in, out);
            };
        }
        case Type::Unknown:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported chain").Flush();

            return {};
        }
    }
}

auto Wallet::Proposals::is_expired(const Proposal& tx) noexcept -> bool
{
    return Clock::now() > Clock::from_time_t(tx.expires());
}

auto Wallet::Proposals::rebroadcast(const Lock& lock) noexcept -> void
{
    // TODO monitor inv messages from peers and wait until a peer
    // we didn't transmit the transaction to tells us about it
    constexpr auto limit = std::chrono::minutes(1);

    for (auto& [id, time] : confirming_) {
        if ((Clock::now() - time) < limit) { continue; }

        auto proposal = db_.LoadProposal(id);

        if (false == proposal.has_value()) { continue; }

        auto pTx = factory::BitcoinTransaction(
            api_, blockchain_, proposal.value().finished());

        if (false == bool(pTx)) { continue; }

        const auto& tx = *pTx;
        network_.BroadcastTransaction(tx);
    }
}

auto Wallet::Proposals::Run() noexcept -> bool
{
    Lock lock{lock_};
    send(lock);
    rebroadcast(lock);
    cleanup(lock);

    return 0 < pending_.size();
}

auto Wallet::Proposals::send(const Lock& lock) noexcept -> void
{
    for (auto it{pending_.begin()}; it != pending_.end();) {
        auto wipe{false};
        auto erase{false};
        auto loop = ScopeGuard{[&]() {
            const auto& [id, promise] = *it;

            if (wipe) {
                db_.CancelProposal(id);
                db_.DeleteProposal(id);
                erase = true;
            }

            if (erase) {
                it = pending_.erase(it);
            } else {
                ++it;
            }
        }};
        auto& [id, promise] = *it;
        auto serialized = db_.LoadProposal(id);

        if (false == serialized.has_value()) { continue; }

        auto& data = *serialized;

        if (is_expired(data)) {
            wipe = true;

            continue;
        }

        if (auto builder = get_builder(); builder) {
            switch (builder(id, data, promise)) {
                case BuildResult::PermanentFailure: {
                    wipe = true;
                    [[fallthrough]];
                }
                case BuildResult::TemporaryFailure: {

                    continue;
                }
                case BuildResult::Success:
                default: {
                    confirming_.emplace(std::move(id), Clock::now());
                    erase = true;
                }
            }
        }
    }
}
}  // namespace opentxs::blockchain::client::implementation

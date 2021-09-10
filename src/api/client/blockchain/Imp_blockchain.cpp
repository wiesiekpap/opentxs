// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "api/client/blockchain/Imp_blockchain.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <string_view>

#include "blockchain/database/common/Database.hpp"
#include "core/Worker.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/api/network/Network.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"  // IWYU pragma: keep
#include "opentxs/protobuf/StorageThread.pb.h"
#include "opentxs/protobuf/StorageThreadItem.pb.h"
#include "opentxs/util/WorkType.hpp"
#include "util/Container.hpp"
#include "util/Work.hpp"

#define OT_METHOD "opentxs::api::client::implementation::BlockchainImp::"

namespace opentxs::api::client::implementation
{
BlockchainImp::BlockchainImp(
    const api::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const Options& args,
    api::client::internal::Blockchain& parent) noexcept
    : Imp(api, contacts, parent)
    , parent_(parent)
    , activity_(activity)
    , key_generated_endpoint_(opentxs::network::zeromq::socket::implementation::
                                  Socket::random_inproc_endpoint())
    , transaction_updates_([&] {
        auto out = api_.Network().ZeroMQ().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainTransactions());

        OT_ASSERT(listen);

        return out;
    }())
    , key_updates_([&] {
        auto out = api_.Network().ZeroMQ().PublishSocket();
        const auto listen = out->Start(key_generated_endpoint_);

        OT_ASSERT(listen);

        return out;
    }())
    , scan_updates_([&] {
        auto out = api_.Network().ZeroMQ().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainScanProgress());

        OT_ASSERT(listen);

        return out;
    }())
    , new_blockchain_accounts_([&] {
        auto out = api_.Network().ZeroMQ().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainAccountCreated());

        OT_ASSERT(listen);

        return out;
    }())
    , balances_(api_)
{
}

auto BlockchainImp::ActivityDescription(
    const identifier::Nym& nym,
    const Identifier& thread,
    const std::string& itemID) const noexcept -> std::string
{
    auto data = proto::StorageThread{};

    if (false == api_.Storage().Load(nym.str(), thread.str(), data)) {
        LogOutput(OT_METHOD)(__func__)(": thread ")(thread.str())(
            " does not exist for nym ")(nym.str())
            .Flush();

        return {};
    }

    for (const auto& item : data.item()) {
        if (item.id() != itemID) { continue; }

        const auto txid = api_.Factory().Data(item.txid(), StringStyle::Raw);
        const auto chain = static_cast<opentxs::blockchain::Type>(item.chain());
        const auto pTx = LoadTransactionBitcoin(txid);

        if (false == bool(pTx)) {
            LogOutput(OT_METHOD)(__func__)(": failed to load transaction ")(
                txid->asHex())
                .Flush();

            return {};
        }

        const auto& tx = *pTx;

        return this->ActivityDescription(nym, chain, tx);
    }

    LogOutput(OT_METHOD)(__func__)(": item ")(itemID)(" not found ").Flush();

    return {};
}

auto BlockchainImp::ActivityDescription(
    const identifier::Nym& nym,
    const opentxs::blockchain::Type chain,
    const Tx& tx) const noexcept -> std::string
{
    auto output = std::stringstream{};
    const auto amount = tx.NetBalanceChange(parent_, nym);
    const auto memo = tx.Memo(parent_);
    const auto& contactAPI = Contacts();
    const auto names = [&] {
        auto out = std::set<std::string>{};
        const auto contacts =
            tx.AssociatedRemoteContacts(parent_, contactAPI, nym);

        for (const auto& id : contacts) {
            out.emplace(contactAPI.ContactName(id, Translate(chain)));
        }

        return out;
    }();

    if (0 < amount) {
        output << "Incoming ";
    } else if (0 > amount) {
        output << "Outgoing ";
    }

    output << opentxs::blockchain::DisplayString(chain);
    output << " transaction";

    if (0 < names.size()) {
        output << " ";

        if (0 < amount) {
            output << "from ";
        } else {
            output << "to ";
        }

        auto n = std::size_t{0};
        const auto max = names.size();

        for (auto i = names.begin(); i != names.end(); ++n, ++i) {
            if (0u == n) {
                output << *i;
            } else if ((n + 1u) == max) {
                output << ", and ";
                output << *i;
            } else {
                output << ", ";
                output << *i;
            }
        }
    }

    if (false == memo.empty()) { output << ": " << memo; }

    return output.str();
}

auto BlockchainImp::AssignTransactionMemo(
    const TxidHex& id,
    const std::string& label) const noexcept -> bool
{
    auto lock = Lock{lock_};
    auto pTransaction = load_transaction(lock, id);

    if (false == bool(pTransaction)) { return false; }

    auto& transaction = *pTransaction;
    transaction.SetMemo(label);
    const auto serialized = transaction.Serialize(parent_);

    OT_ASSERT(serialized.has_value());

    if (false ==
        api_.Network().Blockchain().Internal().Database().StoreTransaction(
            serialized.value())) {
        return false;
    }

    broadcast_update_signal(transaction);

    return true;
}

auto BlockchainImp::broadcast_update_signal(
    const std::vector<pTxid>& transactions) const noexcept -> void
{
    std::for_each(
        std::begin(transactions),
        std::end(transactions),
        [this](const auto& txid) {
            const auto data = api_.Network()
                                  .Blockchain()
                                  .Internal()
                                  .Database()
                                  .LoadTransaction(txid->Bytes());

            OT_ASSERT(data.has_value());

            const auto tx =
                factory::BitcoinTransaction(api_, parent_, data.value());

            OT_ASSERT(tx);

            broadcast_update_signal(*tx);
        });
}

auto BlockchainImp::broadcast_update_signal(
    const opentxs::blockchain::block::bitcoin::internal::Transaction& tx)
    const noexcept -> void
{
    const auto chains = tx.Chains();
    std::for_each(std::begin(chains), std::end(chains), [&](const auto& chain) {
        auto out = api_.Network().ZeroMQ().TaggedMessage(
            WorkType::BlockchainNewTransaction);
        out->AddFrame(tx.ID());
        out->AddFrame(chain);
        transaction_updates_->Send(out);
    });
}

auto BlockchainImp::IndexItem(const ReadView bytes) const noexcept -> PatternID
{
    auto output = PatternID{};
    const auto hashed = api_.Crypto().Hash().HMAC(
        opentxs::crypto::HashType::SipHash24,
        api_.Network().Blockchain().Internal().Database().HashKey(),
        bytes,
        preallocated(sizeof(output), &output));

    OT_ASSERT(hashed);

    return output;
}

auto BlockchainImp::KeyEndpoint() const noexcept -> const std::string&
{
    return key_generated_endpoint_;
}

auto BlockchainImp::KeyGenerated(
    const opentxs::blockchain::Type chain) const noexcept -> void
{
    auto work = MakeWork(api_, OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL);
    work->AddFrame(chain);
    key_updates_->Send(work);
}

auto BlockchainImp::LoadTransactionBitcoin(const TxidHex& txid) const noexcept
    -> std::unique_ptr<const Tx>
{
    auto lock = Lock{lock_};

    return load_transaction(lock, txid);
}

auto BlockchainImp::LoadTransactionBitcoin(const Txid& txid) const noexcept
    -> std::unique_ptr<const Tx>
{
    auto lock = Lock{lock_};

    return load_transaction(lock, txid);
}

auto BlockchainImp::load_transaction(const Lock& lock, const TxidHex& txid)
    const noexcept -> std::unique_ptr<
        opentxs::blockchain::block::bitcoin::internal::Transaction>
{
    return load_transaction(lock, api_.Factory().Data(txid, StringStyle::Hex));
}

auto BlockchainImp::load_transaction(const Lock& lock, const Txid& txid)
    const noexcept -> std::unique_ptr<
        opentxs::blockchain::block::bitcoin::internal::Transaction>
{
    const auto serialized =
        api_.Network().Blockchain().Internal().Database().LoadTransaction(
            txid.Bytes());

    if (false == serialized.has_value()) {
        LogOutput(OT_METHOD)(__func__)(": Transaction ")(txid.asHex())(
            " not found")
            .Flush();

        return {};
    }

    return factory::BitcoinTransaction(api_, parent_, serialized.value());
}

auto BlockchainImp::LookupContacts(const Data& pubkeyHash) const noexcept
    -> ContactList
{
    return api_.Network().Blockchain().Internal().Database().LookupContact(
        pubkeyHash);
}

auto BlockchainImp::notify_new_account(
    const Identifier& id,
    const identifier::Nym& owner,
    opentxs::blockchain::Type chain,
    opentxs::blockchain::crypto::SubaccountType type) const noexcept -> void
{
    {
        auto work = api_.Network().ZeroMQ().TaggedMessage(
            WorkType::BlockchainAccountCreated);
        work->AddFrame(chain);
        work->AddFrame(owner);
        work->AddFrame(type);
        work->AddFrame(id);
        new_blockchain_accounts_->Send(work);
    }

    balances_.RefreshBalance(owner, chain);
}

auto BlockchainImp::ProcessContact(const Contact& contact) const noexcept
    -> bool
{
    broadcast_update_signal(
        api_.Network().Blockchain().Internal().Database().UpdateContact(
            contact));

    return true;
}

auto BlockchainImp::ProcessMergedContact(
    const Contact& parent,
    const Contact& child) const noexcept -> bool
{
    broadcast_update_signal(
        api_.Network().Blockchain().Internal().Database().UpdateMergedContact(
            parent, child));

    return true;
}

auto BlockchainImp::ProcessTransaction(
    const opentxs::blockchain::Type chain,
    const Tx& in,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = Lock{lock_};
    auto pTransaction = in.clone();

    OT_ASSERT(pTransaction);

    auto& transaction = *pTransaction;
    const auto& id = transaction.ID();
    const auto txid = id.Bytes();

    if (const auto tx =
            api_.Network().Blockchain().Internal().Database().LoadTransaction(
                txid);
        tx.has_value()) {
        transaction.MergeMetadata(parent_, chain, tx.value());
        auto updated = transaction.Serialize(parent_);

        OT_ASSERT(updated.has_value());

        if (false ==
            api_.Network().Blockchain().Internal().Database().StoreTransaction(
                updated.value())) {
            return false;
        }
    } else {
        auto serialized = transaction.Serialize(parent_);

        OT_ASSERT(serialized.has_value());

        if (false ==
            api_.Network().Blockchain().Internal().Database().StoreTransaction(
                serialized.value())) {
            return false;
        }
    }

    if (false ==
        api_.Network().Blockchain().Internal().Database().AssociateTransaction(
            id, transaction.GetPatterns())) {
        return false;
    }

    return reconcile_activity_threads(lock, transaction);
}

auto BlockchainImp::reconcile_activity_threads(
    const Lock& lock,
    const Txid& txid) const noexcept -> bool
{
    const auto tx = load_transaction(lock, txid);

    if (false == bool(tx)) { return false; }

    return reconcile_activity_threads(lock, *tx);
}

auto BlockchainImp::reconcile_activity_threads(
    const Lock& lock,
    const opentxs::blockchain::block::bitcoin::internal::Transaction& tx)
    const noexcept -> bool
{
    if (!activity_.AddBlockchainTransaction(parent_, tx)) { return false; }

    broadcast_update_signal(tx);

    return true;
}

auto BlockchainImp::ReportScan(
    const opentxs::blockchain::Type chain,
    const identifier::Nym& owner,
    const opentxs::blockchain::crypto::SubaccountType type,
    const Identifier& id,
    const Blockchain::Subchain subchain,
    const opentxs::blockchain::block::Position& progress) const noexcept -> void
{
    OT_ASSERT(false == owner.empty());
    OT_ASSERT(false == id.empty());

    const auto bytes = id.Bytes();
    const auto hash = progress.second->Bytes();
    auto work = api_.Network().ZeroMQ().TaggedMessage(
        WorkType::BlockchainWalletScanProgress);
    work->AddFrame(chain);
    work->AddFrame(owner.data(), owner.size());
    work->AddFrame(type);
    work->AddFrame(bytes.data(), bytes.size());
    work->AddFrame(subchain);
    work->AddFrame(progress.first);
    work->AddFrame(hash.data(), hash.size());
    scan_updates_->Send(work);
}

auto BlockchainImp::UpdateBalance(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::Balance balance) const noexcept -> void
{
    balances_.UpdateBalance(chain, balance);
}

auto BlockchainImp::UpdateBalance(
    const identifier::Nym& owner,
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::Balance balance) const noexcept -> void
{
    balances_.UpdateBalance(owner, chain, balance);
}

auto BlockchainImp::UpdateElement(std::vector<ReadView>& hashes) const noexcept
    -> void
{
    auto patterns = std::vector<PatternID>{};
    std::for_each(std::begin(hashes), std::end(hashes), [&](const auto& bytes) {
        patterns.emplace_back(IndexItem(bytes));
    });
    LogTrace(OT_METHOD)(__func__)(": ")(patterns.size())(
        " pubkey hashes have changed:")
        .Flush();
    auto transactions = std::vector<pTxid>{};
    std::for_each(
        std::begin(patterns), std::end(patterns), [&](const auto& pattern) {
            LogTrace("    * ")(pattern).Flush();
            auto matches = api_.Network()
                               .Blockchain()
                               .Internal()
                               .Database()
                               .LookupTransactions(pattern);
            transactions.reserve(transactions.size() + matches.size());
            std::move(
                std::begin(matches),
                std::end(matches),
                std::back_inserter(transactions));
        });
    dedup(transactions);
    auto lock = Lock{lock_};
    std::for_each(
        std::begin(transactions),
        std::end(transactions),
        [&](const auto& txid) { reconcile_activity_threads(lock, txid); });
}
}  // namespace opentxs::api::client::implementation

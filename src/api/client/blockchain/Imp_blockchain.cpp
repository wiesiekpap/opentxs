// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "api/client/blockchain/Imp_blockchain.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <future>
#include <iosfwd>
#include <iterator>
#include <optional>
#include <sstream>
#include <string_view>
#include <utility>

#include "api/client/blockchain/BalanceLists.hpp"
#include "api/client/blockchain/SyncClient.hpp"
#include "api/client/blockchain/SyncServer.hpp"
#include "core/Worker.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/ThreadPool.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
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
    const api::internal::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const ArgList& args,
    api::client::internal::Blockchain& parent) noexcept
    : Imp(api, contacts, parent)
    , parent_(parent)
    , activity_(activity)
    , key_generated_endpoint_(opentxs::network::zeromq::socket::implementation::
                                  Socket::random_inproc_endpoint())
    , db_(api_, parent_, legacy, dataFolder, args)
    , reorg_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto listen = out->Start(api_.Endpoints().BlockchainReorg());

        OT_ASSERT(listen);

        return out;
    }())
    , transaction_updates_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainTransactions());

        OT_ASSERT(listen);

        return out;
    }())
    , connected_peer_updates_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainPeerConnection());

        OT_ASSERT(listen);

        return out;
    }())
    , active_peer_updates_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto listen = out->Start(api_.Endpoints().BlockchainPeer());

        OT_ASSERT(listen);

        return out;
    }())
    , key_updates_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto listen = out->Start(key_generated_endpoint_);

        OT_ASSERT(listen);

        return out;
    }())
    , sync_updates_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainSyncProgress());

        OT_ASSERT(listen);

        return out;
    }())
    , scan_updates_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainScanProgress());

        OT_ASSERT(listen);

        return out;
    }())
    , new_blockchain_accounts_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainAccountCreated());

        OT_ASSERT(listen);

        return out;
    }())
    , new_filters_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto listen = out->Start(api_.Endpoints().BlockchainNewFilter());

        OT_ASSERT(listen);

        return out;
    }())
    , block_download_queue_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainBlockDownloadQueue());

        OT_ASSERT(listen);

        return out;
    }())
    , base_config_([&] {
        auto output = opentxs::blockchain::node::internal::Config{};
        const auto sync = [&] {
            try {
                const auto& arg = args.at(OPENTXS_ARG_BLOCKCHAIN_SYNC);

                if (0 == arg.size()) { return false; }

                output.sync_endpoint_ = *arg.begin();

                return true;
            } catch (...) {

                return false;
            }
        }();

        using Policy = api::client::blockchain::BlockStorage;

        if (Policy::All == db_.BlockPolicy()) {
            output.generate_cfilters_ = true;

            if (sync) {
                output.provide_sync_server_ = true;
                output.disable_wallet_ = true;
            }
        } else if (sync) {
            output.use_sync_server_ = true;

        } else {
            output.download_cfilters_ = true;
        }

        try {
            const auto& arg = args.at("disableblockchainwallet");

            if (0 < arg.size()) { output.disable_wallet_ = true; }
        } catch (...) {
        }

        return output;
    }())
    , config_()
    , networks_()
    , sync_client_([&]() -> std::unique_ptr<blockchain::SyncClient> {
        if (base_config_.use_sync_server_) {
            return std::make_unique<blockchain::SyncClient>(api_, parent_);
        }

        return {};
    }())
    , sync_server_([&]() -> std::unique_ptr<blockchain::SyncServer> {
        if (base_config_.provide_sync_server_) {
            return std::make_unique<blockchain::SyncServer>(api_, parent_);
        }

        return {};
    }())
    , balances_(parent_, api_)
    , chain_state_publisher_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        auto rc = out->Start(api_.Endpoints().BlockchainStateChange());

        OT_ASSERT(rc);

        return out;
    }())
    , last_hello_([&] {
        auto output = LastHello{};

        for (const auto& chain : opentxs::blockchain::SupportedChains()) {
            output.emplace(chain, Clock::from_time_t(0));
        }

        return output;
    }())
    , running_(true)
    , heartbeat_(&BlockchainImp::heartbeat, this)
{
    using Work = api::internal::ThreadPool::Work;
    using Wallet = opentxs::blockchain::node::internal::Wallet;
    using Filters = opentxs::blockchain::node::internal::FilterOracle;
    constexpr auto value = [](auto work) {
        return static_cast<OTZMQWorkType>(work);
    };
    const auto& pool = api_.ThreadPool();
    pool.Register(value(Work::BlockchainWallet), [](const auto& work) {
        Wallet::ProcessThreadPool(work);
    });
    pool.Register(value(Work::SyncDataFiltersIncoming), [](const auto& work) {
        Filters::ProcessThreadPool(work);
    });
    pool.Register(value(Work::CalculateBlockFilters), [](const auto& work) {
        Filters::ProcessThreadPool(work);
    });

    try {
        const auto& endpoints = args.at(OPENTXS_ARG_BLOCKCHAIN_SYNC);

        for (const auto& endpoint : endpoints) { AddSyncServer(endpoint); }
    } catch (...) {
    }
}

auto BlockchainImp::ActivityDescription(
    const identifier::Nym& nym,
    const Identifier& thread,
    const std::string& itemID) const noexcept -> std::string
{
    auto data = proto::StorageThread{};

    if (false == api_.Storage().Load(nym.str(), thread.str(), data)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": thread ")(thread.str())(
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
            LogOutput(OT_METHOD)(__FUNCTION__)(": failed to load transaction ")(
                txid->asHex())
                .Flush();

            return {};
        }

        const auto& tx = *pTx;

        return this->ActivityDescription(nym, chain, tx);
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": item ")(itemID)(" not found ")
        .Flush();

    return {};
}

auto BlockchainImp::ActivityDescription(
    const identifier::Nym& nym,
    const Chain chain,
    const Tx& transaction) const noexcept -> std::string
{
    auto output = std::stringstream{};
    const auto amount = transaction.NetBalanceChange(parent_, nym);
    const auto memo = transaction.Memo(parent_);

    if (0 < amount) {
        output << "Incoming ";
    } else if (0 > amount) {
        output << "Outgoing ";
    }

    output << opentxs::blockchain::DisplayString(chain);
    output << " transaction";

    if (false == memo.empty()) { output << ": " << memo; }

    return output.str();
}

auto BlockchainImp::AddSyncServer(const std::string& endpoint) const noexcept
    -> bool
{
    return db_.AddSyncServer(endpoint);
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

    if (false == db_.StoreTransaction(serialized.value())) { return false; }

    broadcast_update_signal(transaction);

    return true;
}

auto BlockchainImp::BlockchainDB() const noexcept
    -> const blockchain::database::implementation::Database&
{
    return db_;
}

auto BlockchainImp::BlockQueueUpdate() const noexcept
    -> const zmq::socket::Publish&
{
    return block_download_queue_;
}

auto BlockchainImp::broadcast_update_signal(
    const std::vector<pTxid>& transactions) const noexcept -> void
{
    std::for_each(
        std::begin(transactions),
        std::end(transactions),
        [this](const auto& txid) {
            const auto data = db_.LoadTransaction(txid->Bytes());

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
        auto out =
            api_.ZeroMQ().TaggedMessage(WorkType::BlockchainNewTransaction);
        out->AddFrame(tx.ID());
        out->AddFrame(chain);
        transaction_updates_->Send(out);
    });
}

auto BlockchainImp::check_hello(const Lock&) const noexcept -> Chains
{
    constexpr auto limit = std::chrono::seconds(30);
    auto output = Chains{};
    const auto now = Clock::now();

    for (const auto& [chain, network] : networks_) {
        auto& last = last_hello_[chain];

        if ((now - last) > limit) {
            last = now;
            output.emplace_back(chain);
        }
    }

    return output;
}

auto BlockchainImp::DeleteSyncServer(const std::string& endpoint) const noexcept
    -> bool
{
    return db_.DeleteSyncServer(endpoint);
}

auto BlockchainImp::Disable(const Chain type) const noexcept -> bool
{
    auto lock = Lock{lock_};

    return disable(lock, type);
}

auto BlockchainImp::disable(const Lock& lock, const Chain type) const noexcept
    -> bool
{
    if (0 == opentxs::blockchain::SupportedChains().count(type)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported chain").Flush();

        return false;
    }

    stop(lock, type);

    if (db_.Disable(type)) { return true; }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Database update failure").Flush();

    return false;
}

auto BlockchainImp::Enable(const Chain type, const std::string& seednode)
    const noexcept -> bool
{
    auto lock = Lock{lock_};

    return enable(lock, type, seednode);
}

auto BlockchainImp::enable(
    const Lock& lock,
    const Chain type,
    const std::string& seednode) const noexcept -> bool
{
    if (0 == opentxs::blockchain::SupportedChains().count(type)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported chain").Flush();

        return false;
    }

    if (false == db_.Enable(type, seednode)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Database error").Flush();

        return false;
    }

    return start(lock, type, seednode);
}

auto BlockchainImp::EnabledChains() const noexcept -> std::set<Chain>
{
    auto out = std::set<Chain>{};
    const auto data = [&] {
        auto lock = Lock{lock_};

        return db_.LoadEnabledChains();
    }();
    std::transform(
        data.begin(),
        data.end(),
        std::inserter(out, out.begin()),
        [](const auto value) { return value.first; });

    return out;
}

auto BlockchainImp::FilterUpdate() const noexcept -> const zmq::socket::Publish&
{
    return new_filters_;
}

auto BlockchainImp::GetChain(const Chain type) const noexcept(false)
    -> const opentxs::blockchain::node::Manager&
{
    auto lock = Lock{lock_};

    return *networks_.at(type);
}

auto BlockchainImp::GetSyncServers() const noexcept -> Blockchain::Endpoints
{
    return db_.GetSyncServers();
}

auto BlockchainImp::heartbeat() const noexcept -> void
{
    while (running_) {
        auto counter{-1};

        while (running_ && (20 > ++counter)) {
            Sleep(std::chrono::milliseconds{250});
        }

        auto lock = Lock{lock_};

        for (const auto& [key, value] : networks_) {
            if (false == running_) { return; }

            value->Heartbeat();
        }
    }
}

auto BlockchainImp::Hello() const noexcept -> SyncState
{
    auto lock = Lock{lock_};
    auto chains = [&] {
        auto output = Chains{};

        for (const auto& [chain, network] : networks_) {
            output.emplace_back(chain);
        }

        return output;
    }();

    return hello(lock, chains);
}

auto BlockchainImp::hello(const Lock&, const Chains& chains) const noexcept
    -> SyncState
{
    auto output = SyncState{};

    for (const auto chain : chains) {
        const auto& network = networks_.at(chain);
        const auto& filter = network->FilterOracle();
        const auto type = filter.DefaultType();
        output.emplace_back(chain, filter.FilterTip(type));
    }

    return output;
}

auto BlockchainImp::IndexItem(const ReadView bytes) const noexcept -> PatternID
{
    auto output = PatternID{};
    const auto hashed = api_.Crypto().Hash().HMAC(
        opentxs::crypto::HashType::SipHash24,
        db_.HashKey(),
        bytes,
        preallocated(sizeof(output), &output));

    OT_ASSERT(hashed);

    return output;
}

auto BlockchainImp::Init() noexcept -> void
{
    Blockchain::Imp::Init();

    if (sync_client_) { sync_client_->Init(); }
}

auto BlockchainImp::IsEnabled(
    const opentxs::blockchain::Type chain) const noexcept -> bool
{
    auto lock = Lock{lock_};

    for (const auto& [enabled, peer] : db_.LoadEnabledChains()) {
        if (chain == enabled) { return true; }
    }

    return false;
}

auto BlockchainImp::KeyEndpoint() const noexcept -> const std::string&
{
    return key_generated_endpoint_;
}

auto BlockchainImp::KeyGenerated(const Chain chain) const noexcept -> void
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
    const auto serialized = db_.LoadTransaction(txid.Bytes());

    if (false == serialized.has_value()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Transaction ")(txid.asHex())(
            " not found")
            .Flush();

        return {};
    }

    return factory::BitcoinTransaction(api_, parent_, serialized.value());
}

auto BlockchainImp::LookupContacts(const Data& pubkeyHash) const noexcept
    -> ContactList
{
    return db_.LookupContact(pubkeyHash);
}

auto BlockchainImp::notify_new_account(
    const Identifier& id,
    const identifier::Nym& owner,
    Chain chain,
    Blockchain::AccountType type) const noexcept -> void
{
    {
        auto work =
            api_.ZeroMQ().TaggedMessage(WorkType::BlockchainAccountCreated);
        work->AddFrame(chain);
        work->AddFrame(owner);
        work->AddFrame(type);
        work->AddFrame(id);
        new_blockchain_accounts_->Send(work);
    }

    balances_.RefreshBalance(owner, chain);
}

auto BlockchainImp::PeerUpdate() const noexcept -> const zmq::socket::Publish&
{
    return connected_peer_updates_;
}

auto BlockchainImp::ProcessContact(const Contact& contact) const noexcept
    -> bool
{
    broadcast_update_signal(db_.UpdateContact(contact));

    return true;
}

auto BlockchainImp::ProcessMergedContact(
    const Contact& parent,
    const Contact& child) const noexcept -> bool
{
    broadcast_update_signal(db_.UpdateMergedContact(parent, child));

    return true;
}

auto BlockchainImp::ProcessTransaction(
    const Chain chain,
    const Tx& in,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = Lock{lock_};
    auto pTransaction = in.clone();

    OT_ASSERT(pTransaction);

    auto& transaction = *pTransaction;
    const auto& id = transaction.ID();
    const auto txid = id.Bytes();

    if (const auto tx = db_.LoadTransaction(txid); tx.has_value()) {
        transaction.MergeMetadata(parent_, chain, tx.value());
        auto updated = transaction.Serialize(parent_);

        OT_ASSERT(updated.has_value());

        if (false == db_.StoreTransaction(updated.value())) { return false; }
    } else {
        auto serialized = transaction.Serialize(parent_);

        OT_ASSERT(serialized.has_value());

        if (false == db_.StoreTransaction(serialized.value())) { return false; }
    }

    if (false == db_.AssociateTransaction(id, transaction.GetPatterns())) {
        return false;
    }

    return reconcile_activity_threads(lock, transaction);
}

auto BlockchainImp::publish_chain_state(Chain type, bool state) const -> void
{
    auto work = api_.ZeroMQ().TaggedMessage(WorkType::BlockchainStateChange);
    work->AddFrame(type);
    work->AddFrame(state);
    chain_state_publisher_->Send(work);
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

auto BlockchainImp::Reorg() const noexcept -> const zmq::socket::Publish&
{
    return reorg_;
}

auto BlockchainImp::ReportProgress(
    const Chain chain,
    const opentxs::blockchain::block::Height current,
    const opentxs::blockchain::block::Height target) const noexcept -> void
{
    auto work = api_.ZeroMQ().TaggedMessage(WorkType::BlockchainSyncProgress);
    work->AddFrame(chain);
    work->AddFrame(current);
    work->AddFrame(target);
    sync_updates_->Send(work);
}

auto BlockchainImp::ReportScan(
    const Chain chain,
    const identifier::Nym& owner,
    const Identifier& account,
    const blockchain::Subchain subchain,
    const opentxs::blockchain::block::Position& progress) const noexcept -> void
{
    OT_ASSERT(false == owner.empty());
    OT_ASSERT(false == account.empty());

    const auto id = account.Bytes();
    const auto hash = progress.second->Bytes();
    auto work =
        api_.ZeroMQ().TaggedMessage(WorkType::BlockchainWalletScanProgress);
    work->AddFrame(chain);
    work->AddFrame(owner.data(), owner.size());
    work->AddFrame(id.data(), id.size());
    work->AddFrame(subchain);
    work->AddFrame(progress.first);
    work->AddFrame(hash.data(), hash.size());
    scan_updates_->Send(work);
}

auto BlockchainImp::RestoreNetworks() const noexcept -> void
{
    for (const auto& [chain, peer] : db_.LoadEnabledChains()) {
        Start(chain, peer);
    }
}

auto BlockchainImp::Shutdown() noexcept -> void
{
    if (running_.exchange(false)) {
        if (heartbeat_.joinable()) { heartbeat_.join(); }

        LogVerbose("Shutting down ")(networks_.size())(" blockchain clients")
            .Flush();

        for (auto& [chain, network] : networks_) { network->Shutdown().get(); }

        networks_.clear();
    }

    Blockchain::Imp::Shutdown();
}

auto BlockchainImp::Start(const Chain type, const std::string& seednode)
    const noexcept -> bool
{
    auto lock = Lock{lock_};

    return start(lock, type, seednode);
}

auto BlockchainImp::start(
    const Lock& lock,
    const Chain type,
    const std::string& seednode) const noexcept -> bool
{
    if (Chain::UnitTest != type) {
        if (0 == opentxs::blockchain::SupportedChains().count(type)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported chain").Flush();

            return false;
        }
    }

    if (0 != networks_.count(type)) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Chain already running").Flush();

        return true;
    }

    namespace p2p = opentxs::blockchain::p2p;

    switch (
        opentxs::blockchain::params::Data::Chains().at(type).p2p_protocol_) {
        case p2p::Protocol::bitcoin: {
            auto endpoint = std::string{};

            if (sync_server_) {
                sync_server_->Enable(type);
                endpoint = sync_server_->Endpoint(type);
            }

            auto& config = [&]() -> const Config& {
                {
                    auto it = config_.find(type);

                    if (config_.end() != it) { return it->second; }
                }

                auto [it, added] = config_.emplace(type, base_config_);

                OT_ASSERT(added);

                return it->second;
            }();

            auto [it, added] = networks_.emplace(
                type,
                factory::BlockchainNetworkBitcoin(
                    api_, parent_, type, config, seednode, endpoint));
            LogVerbose(OT_METHOD)(__FUNCTION__)(": started chain ")(
                static_cast<std::uint32_t>(type))
                .Flush();
            balance_lists_.Get(type);
            publish_chain_state(type, true);

            return it->second->Connect();
        }
        case p2p::Protocol::opentxs:
        case p2p::Protocol::ethereum:
        default: {
        }
    }

    return false;
}

auto BlockchainImp::StartSyncServer(
    const std::string& sync,
    const std::string& publicSync,
    const std::string& update,
    const std::string& publicUpdate) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (sync_server_) {
        return sync_server_->Start(sync, publicSync, update, publicUpdate);
    }

    LogNormal("Blockchain sync server must be enabled at library "
              "initialization time by passing the ")(
        OPENTXS_ARG_BLOCKCHAIN_SYNC)(" option.")
        .Flush();

    return false;
}

auto BlockchainImp::Stop(const Chain type) const noexcept -> bool
{
    auto lock = Lock{lock_};

    return stop(lock, type);
}

auto BlockchainImp::stop(const Lock& lock, const Chain type) const noexcept
    -> bool
{
    auto it = networks_.find(type);

    if (networks_.end() == it) { return true; }

    OT_ASSERT(it->second);

    if (sync_server_) { sync_server_->Disable(type); }

    it->second->Shutdown().get();
    networks_.erase(it);
    LogVerbose(OT_METHOD)(__FUNCTION__)(": stopped chain ")(
        static_cast<std::uint32_t>(type))
        .Flush();
    publish_chain_state(type, false);

    return true;
}

auto BlockchainImp::SyncEndpoint() const noexcept -> const std::string&
{
    if (sync_client_) {

        return sync_client_->Endpoint();
    } else {

        return Blockchain::Imp::SyncEndpoint();
    }
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
    LogTrace(OT_METHOD)(__FUNCTION__)(": ")(patterns.size())(
        " pubkey hashes have changed:")
        .Flush();
    auto transactions = std::vector<pTxid>{};
    std::for_each(
        std::begin(patterns), std::end(patterns), [&](const auto& pattern) {
            LogTrace("    * ")(pattern).Flush();
            auto matches = db_.LookupTransactions(pattern);
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

auto BlockchainImp::UpdatePeer(
    const opentxs::blockchain::Type chain,
    const std::string& address) const noexcept -> void
{
    auto work = MakeWork(api_, WorkType::BlockchainPeerAdded);
    work->AddFrame(chain);
    work->AddFrame(address);
    active_peer_updates_->Send(work);
}
}  // namespace opentxs::api::client::implementation

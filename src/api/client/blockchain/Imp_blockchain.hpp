// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "api/client/Blockchain.hpp"
#include "api/client/blockchain/Imp.hpp"
#include "api/client/blockchain/SyncClient.hpp"
#include "api/client/blockchain/SyncServer.hpp"
#include "api/client/blockchain/database/Database.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/blockchain/Subchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
struct SyncClient;
struct SyncServer;
}  // namespace blockchain

namespace internal
{
struct Blockchain;
}  // namespace internal

class Activity;
class Contacts;
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal

class Core;
class Legacy;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Transaction;
}  // namespace internal
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace blockchain
{
namespace sync
{
class Data;
}  // namespace sync
}  // namespace blockchain

namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network

class Contact;
class Identifier;
class PasswordPrompt;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::client::implementation
{
struct BalanceOracle {
    using Balance = opentxs::blockchain::Balance;
    using Chain = opentxs::blockchain::Type;

    auto RefreshBalance(const identifier::Nym& owner, const Chain chain)
        const noexcept -> void;
    auto UpdateBalance(const Chain chain, const Balance balance) const noexcept
        -> void;
    auto UpdateBalance(
        const identifier::Nym& owner,
        const Chain chain,
        const Balance balance) const noexcept -> void;

    BalanceOracle(
        const api::client::internal::Blockchain& parent,
        const api::Core& api) noexcept;

private:
    using Subscribers = std::set<OTData>;

    const api::client::internal::Blockchain& parent_;
    const api::Core& api_;
    const zmq::Context& zmq_;
    OTZMQListenCallback cb_;
    OTZMQRouterSocket socket_;
    OTZMQPublishSocket publisher_;
    mutable std::mutex lock_;
    mutable std::map<Chain, Subscribers> subscribers_;
    mutable std::map<Chain, std::map<OTNymID, Subscribers>> nym_subscribers_;

    auto cb(zmq::Message& message) noexcept -> void;

    BalanceOracle() = delete;
};

struct EnableCallbacks {
    using Chain = Blockchain::Chain;
    using EnabledCallback = std::function<bool(const bool)>;

    auto Add(const Chain type, EnabledCallback cb) noexcept -> std::size_t;
    auto Delete(const Chain type, const std::size_t index) noexcept -> void;
    auto Execute(const Chain type, const bool value) noexcept -> void;

    EnableCallbacks(const api::Core& api) noexcept;

private:
    const zmq::Context& zmq_;
    mutable std::mutex lock_;
    std::map<Chain, std::vector<EnabledCallback>> map_;
    OTZMQPublishSocket socket_;

    EnableCallbacks(const EnableCallbacks&) = delete;
    EnableCallbacks(EnableCallbacks&&) = delete;
    auto operator=(const EnableCallbacks&) -> EnableCallbacks& = delete;
    auto operator=(EnableCallbacks&&) -> EnableCallbacks& = delete;
};

struct BlockchainImp final : public Blockchain::Imp {
    using Chain = Blockchain::Chain;
    using Txid = opentxs::blockchain::block::Txid;
    using pTxid = opentxs::blockchain::block::pTxid;
    using LastHello = std::map<Chain, Time>;
    using Chains = std::vector<Chain>;
    using Config = opentxs::blockchain::node::internal::Config;
    using Tx = Blockchain::Tx;
    using TxidHex = Blockchain::TxidHex;
    using PatternID = Blockchain::PatternID;
    using ContactList = Blockchain::ContactList;

    auto ActivityDescription(
        const identifier::Nym& nym,
        const Identifier& thread,
        const std::string& threadItemID) const noexcept -> std::string final;
    auto ActivityDescription(
        const identifier::Nym& nym,
        const Chain chain,
        const Tx& transaction) const noexcept -> std::string final;
    auto AddSyncServer(const std::string& endpoint) const noexcept
        -> bool final;
    auto AssignTransactionMemo(const TxidHex& id, const std::string& label)
        const noexcept -> bool final;
    auto BlockchainDB() const noexcept
        -> const blockchain::database::implementation::Database& final;
    auto BlockQueueUpdate() const noexcept -> const zmq::socket::Publish& final;
    auto DeleteSyncServer(const std::string& endpoint) const noexcept
        -> bool final;
    auto Disable(const Chain type) const noexcept -> bool final;
    auto Enable(const Chain type, const std::string& seednode) const noexcept
        -> bool final;
    auto EnabledChains() const noexcept -> std::set<Chain> final;
    auto FilterUpdate() const noexcept -> const zmq::socket::Publish& final;
    auto GetChain(const Chain type) const noexcept(false)
        -> const opentxs::blockchain::node::Manager& final;
    auto GetSyncServers() const noexcept -> Blockchain::Endpoints final;
    auto Hello() const noexcept -> SyncState final;
    auto IndexItem(const ReadView bytes) const noexcept -> PatternID final;
    auto IsEnabled(const opentxs::blockchain::Type chain) const noexcept
        -> bool final;
    auto KeyEndpoint() const noexcept -> const std::string& final;
    auto KeyGenerated(const Chain chain) const noexcept -> void final;
    auto LoadTransactionBitcoin(const TxidHex& txid) const noexcept
        -> std::unique_ptr<const Tx> final;
    auto LoadTransactionBitcoin(const Txid& txid) const noexcept
        -> std::unique_ptr<const Tx> final;
    auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList final;
    auto PeerUpdate() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& final;
    auto ProcessContact(const Contact& contact) const noexcept -> bool final;
    auto ProcessMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> bool final;
    auto ProcessTransaction(
        const Chain chain,
        const Tx& in,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto Reorg() const noexcept -> const zmq::socket::Publish& final;
    auto ReportProgress(
        const Chain chain,
        const opentxs::blockchain::block::Height current,
        const opentxs::blockchain::block::Height target) const noexcept
        -> void final;
    auto ReportScan(
        const Chain chain,
        const identifier::Nym& owner,
        const Identifier& account,
        const blockchain::Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void final;
    auto RestoreNetworks() const noexcept -> void final;
    auto Start(const Chain type, const std::string& seednode) const noexcept
        -> bool final;
    auto StartSyncServer(
        const std::string& sync,
        const std::string& publicSync,
        const std::string& update,
        const std::string& publicUpdate) const noexcept -> bool final;
    auto Stop(const Chain type) const noexcept -> bool final;
    auto SyncEndpoint() const noexcept -> const std::string& final;
    auto UpdateBalance(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept
        -> void final;
    auto UpdateBalance(
        const identifier::Nym& owner,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept
        -> void final;
    auto UpdateElement(std::vector<ReadView>& pubkeyHashes) const noexcept
        -> void final;
    auto UpdatePeer(
        const opentxs::blockchain::Type chain,
        const std::string& address) const noexcept -> void final;

    auto Init() noexcept -> void final;
    auto Shutdown() noexcept -> void final;

    BlockchainImp(
        const api::internal::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const ArgList& args,
        api::client::internal::Blockchain& parent) noexcept;

    ~BlockchainImp() final = default;

private:
    api::client::internal::Blockchain& parent_;
    const api::client::Activity& activity_;
    const std::string key_generated_endpoint_;
    blockchain::database::implementation::Database db_;
    OTZMQPublishSocket reorg_;
    OTZMQPublishSocket transaction_updates_;
    OTZMQPublishSocket connected_peer_updates_;
    OTZMQPublishSocket active_peer_updates_;
    OTZMQPublishSocket key_updates_;
    OTZMQPublishSocket sync_updates_;
    OTZMQPublishSocket scan_updates_;
    OTZMQPublishSocket new_blockchain_accounts_;
    OTZMQPublishSocket new_filters_;
    OTZMQPublishSocket block_download_queue_;
    const Config base_config_;
    mutable std::map<Chain, Config> config_;
    mutable std::map<
        Chain,
        std::unique_ptr<opentxs::blockchain::node::internal::Network>>
        networks_;
    std::unique_ptr<blockchain::SyncClient> sync_client_;
    std::unique_ptr<blockchain::SyncServer> sync_server_;
    BalanceOracle balances_;
    OTZMQPublishSocket chain_state_publisher_;
    mutable LastHello last_hello_;
    std::atomic_bool running_;
    std::thread heartbeat_;

    auto broadcast_update_signal(const Txid& txid) const noexcept -> void
    {
        broadcast_update_signal(std::vector<pTxid>{txid});
    }
    auto broadcast_update_signal(
        const std::vector<pTxid>& transactions) const noexcept -> void;
    auto broadcast_update_signal(
        const opentxs::blockchain::block::bitcoin::internal::Transaction& tx)
        const noexcept -> void;
    auto check_hello(const Lock& lock) const noexcept -> Chains;
    auto disable(const Lock& lock, const Chain type) const noexcept -> bool;
    auto enable(const Lock& lock, const Chain type, const std::string& seednode)
        const noexcept -> bool;
    auto heartbeat() const noexcept -> void;
    auto hello(const Lock&, const Chains& chains) const noexcept -> SyncState;
    auto load_transaction(const Lock& lock, const Txid& id) const noexcept
        -> std::unique_ptr<
            opentxs::blockchain::block::bitcoin::internal::Transaction>;
    auto load_transaction(const Lock& lock, const TxidHex& id) const noexcept
        -> std::unique_ptr<
            opentxs::blockchain::block::bitcoin::internal::Transaction>;
    auto notify_new_account(
        const Identifier& id,
        const identifier::Nym& owner,
        Chain chain,
        Blockchain::AccountType type) const noexcept -> void final;
    auto publish_chain_state(Chain type, bool state) const -> void;
    auto reconcile_activity_threads(const Lock& lock, const Txid& txid)
        const noexcept -> bool;
    auto reconcile_activity_threads(
        const Lock& lock,
        const opentxs::blockchain::block::bitcoin::internal::Transaction& tx)
        const noexcept -> bool;
    auto start(const Lock& lock, const Chain type, const std::string& seednode)
        const noexcept -> bool;
    auto stop(const Lock& lock, const Chain type) const noexcept -> bool;
};
}  // namespace opentxs::api::client::implementation

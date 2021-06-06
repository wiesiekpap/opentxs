// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "api/client/Blockchain.hpp"
#include "api/client/blockchain/BalanceLists.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/blockchain/BalanceNode.hpp"
#include "opentxs/api/client/blockchain/Subchain.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace database
{
namespace implementation
{
class Database;
}  // namespace implementation
}  // namespace database

namespace internal
{
struct BalanceList;
struct BalanceNode;
struct BalanceTree;
}  // namespace internal

class HD;
class PaymentCode;
}  // namespace blockchain

namespace internal
{
struct Blockchain;
}  // namespace internal

class Contacts;
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal

class Core;
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
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
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace proto
{
class HDPath;
}  // namespace proto

class Contact;
class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::client::implementation
{
struct AccountCache {
    using AccountType = Blockchain::AccountType;
    using Chain = Blockchain::Chain;

    auto List(const identifier::Nym& nymID, const Chain chain) const noexcept
        -> std::set<OTIdentifier>;
    auto New(
        const AccountType type,
        const Chain chain,
        const Identifier& account,
        const identifier::Nym& owner) const noexcept -> void;
    auto Owner(const Identifier& accountID) const noexcept
        -> const identifier::Nym&;
    auto Type(const Identifier& accountID) const noexcept -> AccountType;

    auto Populate() noexcept -> void;

    AccountCache(const api::Core& api) noexcept;

private:
    using NymAccountMap = std::map<OTNymID, std::set<OTIdentifier>>;
    using ChainAccountMap = std::map<Chain, std::optional<NymAccountMap>>;
    using AccountNymIndex = std::map<OTIdentifier, OTNymID>;
    using AccountTypeIndex = std::map<OTIdentifier, AccountType>;

    const api::Core& api_;
    mutable std::mutex lock_;
    mutable ChainAccountMap account_map_;
    mutable AccountNymIndex account_index_;
    mutable AccountTypeIndex account_type_;

    auto build_account_map(
        const Lock&,
        const Chain chain,
        std::optional<NymAccountMap>& map) const noexcept -> void;
    auto get_account_map(const Lock&, const Chain chain) const noexcept
        -> NymAccountMap&;
};

struct Blockchain::Imp {
    using IDLock = std::map<OTIdentifier, std::mutex>;

    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> std::set<OTIdentifier>;
    auto AccountList(const Chain chain) const noexcept
        -> std::set<OTIdentifier>;
    auto AccountList() const noexcept -> std::set<OTIdentifier>;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const Identifier& thread,
        const std::string& threadItemID) const noexcept -> std::string;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const Chain chain,
        const Tx& transaction) const noexcept -> std::string;
    auto address_prefix(const Style style, const Chain chain) const
        noexcept(false) -> OTData;
    virtual auto AddSyncServer(
        [[maybe_unused]] const std::string& endpoint) const noexcept -> bool
    {
        return false;
    }
    auto AssignContact(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Subchain subchain,
        const Bip32Index index,
        const Identifier& contactID) const noexcept -> bool;
    auto AssignLabel(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Subchain subchain,
        const Bip32Index index,
        const std::string& label) const noexcept -> bool;
    virtual auto AssignTransactionMemo(
        const TxidHex& id,
        const std::string& label) const noexcept -> bool
    {
        return false;
    }
    auto BalanceList(const Chain chain) const noexcept(false)
        -> const blockchain::internal::BalanceList&;
    auto BalanceTree(const identifier::Nym& nymID, const Chain chain) const
        noexcept(false) -> const blockchain::internal::BalanceTree&;
    virtual auto BlockchainDB() const noexcept
        -> const blockchain::database::implementation::Database&;
    virtual auto BlockQueueUpdate() const noexcept
        -> const zmq::socket::Publish&;
    auto CalculateAddress(
        const Chain chain,
        const Style format,
        const Data& pubkey) const noexcept -> std::string;
    auto Confirm(
        const blockchain::Key key,
        const opentxs::blockchain::block::Txid& tx) const noexcept -> bool;
    virtual auto ConnectedSyncServers() const noexcept -> Endpoints
    {
        return {};
    }
    auto Contacts() const noexcept -> const api::client::Contacts&
    {
        return contacts_;
    }
    auto DecodeAddress(const std::string& encoded) const noexcept
        -> DecodedAddress;
    virtual auto DeleteSyncServer(
        [[maybe_unused]] const std::string& endpoint) const noexcept -> bool
    {
        return false;
    }
    virtual auto Disable(const Chain type) const noexcept -> bool;
    virtual auto Enable(const Chain type, const std::string& seednode)
        const noexcept -> bool;
    virtual auto EnabledChains() const noexcept -> std::set<Chain>;
    auto EncodeAddress(const Style style, const Chain chain, const Data& data)
        const noexcept -> std::string;
    virtual auto FilterUpdate() const noexcept -> const zmq::socket::Publish&;
    virtual auto GetChain(const Chain type) const noexcept(false)
        -> const opentxs::blockchain::node::Manager&;
    auto GetKey(const blockchain::Key& id) const noexcept(false)
        -> const blockchain::BalanceNode::Element&;
    virtual auto GetSyncServers() const noexcept -> Endpoints { return {}; }
    auto HDSubaccount(const identifier::Nym& nymID, const Identifier& accountID)
        const noexcept(false) -> const blockchain::HD&;
    using SyncState = std::vector<opentxs::network::blockchain::sync::State>;
    virtual auto Hello() const noexcept -> SyncState;
    virtual auto IndexItem(const ReadView bytes) const noexcept -> PatternID;
    virtual auto IsEnabled(const opentxs::blockchain::Type chain) const noexcept
        -> bool;
    virtual auto KeyEndpoint() const noexcept -> const std::string&;
    virtual auto KeyGenerated(const Chain chain) const noexcept -> void;
    virtual auto LoadTransactionBitcoin(const TxidHex& txid) const noexcept
        -> std::unique_ptr<const Tx>;
    virtual auto LoadTransactionBitcoin(const Txid& txid) const noexcept
        -> std::unique_ptr<const Tx>;
    auto LookupAccount(const Identifier& id) const noexcept -> AccountData;
    virtual auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList;
    auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const BlockchainAccountType standard,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier;
    auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier;
    auto Owner(const Identifier& accountID) const noexcept
        -> const identifier::Nym&
    {
        return accounts_.Owner(accountID);
    }
    auto Owner(const blockchain::Key& key) const noexcept
        -> const identifier::Nym&;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept(false)
        -> const blockchain::PaymentCode&;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept(false)
        -> const blockchain::PaymentCode&;
    virtual auto PeerUpdate() const noexcept
        -> const opentxs::network::zeromq::socket::Publish&;
    virtual auto ProcessContact(const Contact& contact) const noexcept -> bool;
    virtual auto ProcessMergedContact(
        const Contact& parent,
        const Contact& child) const noexcept -> bool;
    virtual auto ProcessTransaction(
        const Chain chain,
        const Tx& in,
        const PasswordPrompt& reason) const noexcept -> bool;
    auto PubkeyHash(const Chain chain, const Data& pubkey) const noexcept(false)
        -> OTData;
    auto RecipientContact(const blockchain::Key& key) const noexcept
        -> OTIdentifier;
    auto Release(const blockchain::Key key) const noexcept -> bool;
    virtual auto Reorg() const noexcept -> const zmq::socket::Publish&;
    virtual auto ReportProgress(
        const Chain chain,
        const opentxs::blockchain::block::Height current,
        const opentxs::blockchain::block::Height target) const noexcept -> void;
    virtual auto ReportScan(
        const Chain chain,
        const identifier::Nym& owner,
        const Identifier& account,
        const blockchain::Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void;
    virtual auto RestoreNetworks() const noexcept -> void;
    auto SenderContact(const blockchain::Key& key) const noexcept
        -> OTIdentifier;
    virtual auto Start(const Chain type, const std::string& seednode)
        const noexcept -> bool;
    virtual auto StartSyncServer(
        const std::string& sync,
        const std::string& publicSync,
        const std::string& update,
        const std::string& publicUpdate) const noexcept -> bool;
    virtual auto Stop(const Chain type) const noexcept -> bool;
    auto SubaccountList(const identifier::Nym& nymID, const Chain chain)
        const noexcept -> std::set<OTIdentifier>
    {
        return accounts_.List(nymID, chain);
    }
    virtual auto SyncEndpoint() const noexcept -> const std::string&;
    auto Unconfirm(
        const blockchain::Key key,
        const opentxs::blockchain::block::Txid& tx,
        const Time time) const noexcept -> bool;
    virtual auto UpdateBalance(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept -> void;
    virtual auto UpdateBalance(
        const identifier::Nym& owner,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept -> void;
    virtual auto UpdateElement(
        std::vector<ReadView>& pubkeyHashes) const noexcept -> void;
    virtual auto UpdatePeer(
        const opentxs::blockchain::Type chain,
        const std::string& address) const noexcept -> void;

    virtual auto Init() noexcept -> void;
    virtual auto Shutdown() noexcept -> void;

    Imp(const api::internal::Core& api,
        const api::client::Contacts& contacts,
        api::client::internal::Blockchain& parent) noexcept;

    virtual ~Imp() = default;

protected:
    const api::internal::Core& api_;
    const api::client::Contacts& contacts_;
    const DecodedAddress blank_;
    mutable std::mutex lock_;
    mutable IDLock nym_lock_;
    mutable AccountCache accounts_;
    mutable BalanceLists balance_lists_;

    auto bip44_type(const contact::ContactItemType type) const noexcept
        -> Bip44Type;
    auto decode_bech23(const std::string& encoded) const noexcept
        -> std::optional<DecodedAddress>;
    auto decode_legacy(const std::string& encoded) const noexcept
        -> std::optional<DecodedAddress>;
    auto get_node(const Identifier& accountID) const noexcept(false)
        -> blockchain::internal::BalanceNode&;
    auto init_path(
        const std::string& root,
        const contact::ContactItemType chain,
        const Bip32Index account,
        const BlockchainAccountType standard,
        proto::HDPath& path) const noexcept -> void;
    auto new_payment_code(
        const Lock& lock,
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier;
    auto p2pkh(const Chain chain, const Data& pubkeyHash) const noexcept
        -> std::string;
    auto p2sh(const Chain chain, const Data& scriptHash) const noexcept
        -> std::string;
    auto nym_mutex(const identifier::Nym& nym) const noexcept -> std::mutex&;
    auto validate_nym(const identifier::Nym& nymID) const noexcept -> bool;

private:
    virtual auto notify_new_account(
        const Identifier& id,
        const identifier::Nym& owner,
        Chain chain,
        AccountType type) const noexcept -> void
    {
    }
};
}  // namespace opentxs::api::client::implementation

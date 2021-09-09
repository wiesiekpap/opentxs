// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "api/client/blockchain/AccountCache.hpp"
#include "api/client/blockchain/Blockchain.hpp"
#include "api/client/blockchain/Wallets.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
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
namespace internal
{
struct Blockchain;
}  // namespace internal

class Contacts;
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace crypto
{
namespace internal
{
struct Subaccount;
struct Wallet;
}  // namespace internal

class Account;
class Element;
class HD;
class PaymentCode;
class Subaccount;
class Wallet;
}  // namespace crypto

namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

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
struct Blockchain::Imp {
    using IDLock = std::map<OTIdentifier, std::mutex>;

    auto Account(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Account&;
    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> std::set<OTIdentifier>;
    auto AccountList(const opentxs::blockchain::Type chain) const noexcept
        -> std::set<OTIdentifier>;
    auto AccountList() const noexcept -> std::set<OTIdentifier>;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const Identifier& thread,
        const std::string& threadItemID) const noexcept -> std::string;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const opentxs::blockchain::Type chain,
        const Tx& transaction) const noexcept -> std::string;
    auto address_prefix(
        const Style style,
        const opentxs::blockchain::Type chain) const noexcept(false) -> OTData;
    auto AssignContact(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const Identifier& contactID) const noexcept -> bool;
    auto AssignLabel(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const std::string& label) const noexcept -> bool;
    virtual auto AssignTransactionMemo(
        const TxidHex& id,
        const std::string& label) const noexcept -> bool
    {
        return false;
    }
    auto CalculateAddress(
        const opentxs::blockchain::Type chain,
        const Style format,
        const Data& pubkey) const noexcept -> std::string;
    auto Confirm(const Key key, const opentxs::blockchain::block::Txid& tx)
        const noexcept -> bool;
    auto Contacts() const noexcept -> const api::client::Contacts&
    {
        return contacts_;
    }
    auto DecodeAddress(const std::string& encoded) const noexcept
        -> DecodedAddress;
    auto EncodeAddress(
        const Style style,
        const opentxs::blockchain::Type chain,
        const Data& data) const noexcept -> std::string;
    auto GetKey(const Key& id) const noexcept(false)
        -> const opentxs::blockchain::crypto::Element&;
    auto HDSubaccount(const identifier::Nym& nymID, const Identifier& accountID)
        const noexcept(false) -> const opentxs::blockchain::crypto::HD&;
    using SyncState = std::vector<opentxs::network::blockchain::sync::State>;
    virtual auto IndexItem(const ReadView bytes) const noexcept -> PatternID;
    virtual auto KeyEndpoint() const noexcept -> const std::string&;
    virtual auto KeyGenerated(
        const opentxs::blockchain::Type chain) const noexcept -> void;
    virtual auto LoadTransactionBitcoin(const TxidHex& txid) const noexcept
        -> std::unique_ptr<const Tx>;
    virtual auto LoadTransactionBitcoin(const Txid& txid) const noexcept
        -> std::unique_ptr<const Tx>;
    auto LookupAccount(const Identifier& id) const noexcept -> AccountData;
    virtual auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList;
    auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const opentxs::blockchain::Type derivationChain,
        const opentxs::blockchain::Type targetChain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier;
    auto NewNym(const identifier::Nym& id) const noexcept -> void;
    auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const opentxs::blockchain::Type chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier;
    auto Owner(const Identifier& accountID) const noexcept
        -> const identifier::Nym&
    {
        return accounts_.Owner(accountID);
    }
    auto Owner(const Key& key) const noexcept -> const identifier::Nym&;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode&;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const opentxs::blockchain::Type chain,
        const PasswordPrompt& reason) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode&;
    virtual auto ProcessContact(const Contact& contact) const noexcept -> bool;
    virtual auto ProcessMergedContact(
        const Contact& parent,
        const Contact& child) const noexcept -> bool;
    virtual auto ProcessTransaction(
        const opentxs::blockchain::Type chain,
        const Tx& in,
        const PasswordPrompt& reason) const noexcept -> bool;
    auto PubkeyHash(const opentxs::blockchain::Type chain, const Data& pubkey)
        const noexcept(false) -> OTData;
    auto RecipientContact(const Key& key) const noexcept -> OTIdentifier;
    auto Release(const Key key) const noexcept -> bool;
    virtual auto ReportScan(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const Identifier& account,
        const Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void;
    auto SenderContact(const Key& key) const noexcept -> OTIdentifier;
    auto SubaccountList(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain) const noexcept
        -> std::set<OTIdentifier>
    {
        return accounts_.List(nymID, chain);
    }
    auto Unconfirm(
        const Key key,
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
    auto Wallet(const opentxs::blockchain::Type chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Wallet&;

    virtual auto Init() noexcept -> void;

    Imp(const api::Core& api,
        const api::client::Contacts& contacts,
        api::client::internal::Blockchain& parent) noexcept;

    virtual ~Imp() = default;

protected:
    const api::Core& api_;
    const api::client::Contacts& contacts_;
    const DecodedAddress blank_;
    mutable std::mutex lock_;
    mutable IDLock nym_lock_;
    mutable blockchain::AccountCache accounts_;
    mutable blockchain::Wallets wallets_;

    auto bip44_type(const contact::ContactItemType type) const noexcept
        -> Bip44Type;
    auto decode_bech23(const std::string& encoded) const noexcept
        -> std::optional<DecodedAddress>;
    auto decode_legacy(const std::string& encoded) const noexcept
        -> std::optional<DecodedAddress>;
    auto get_node(const Identifier& accountID) const noexcept(false)
        -> opentxs::blockchain::crypto::Subaccount&;
    auto init_path(
        const std::string& root,
        const contact::ContactItemType chain,
        const Bip32Index account,
        const opentxs::blockchain::crypto::HDProtocol standard,
        proto::HDPath& path) const noexcept -> void;
    auto new_payment_code(
        const Lock& lock,
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const opentxs::blockchain::Type chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier;
    auto p2pkh(const opentxs::blockchain::Type chain, const Data& pubkeyHash)
        const noexcept -> std::string;
    auto p2sh(const opentxs::blockchain::Type chain, const Data& scriptHash)
        const noexcept -> std::string;
    auto p2wpkh(const opentxs::blockchain::Type chain, const Data& pubkeyHash)
        const noexcept -> std::string;
    auto nym_mutex(const identifier::Nym& nym) const noexcept -> std::mutex&;
    auto validate_nym(const identifier::Nym& nymID) const noexcept -> bool;

private:
    virtual auto notify_new_account(
        const Identifier& id,
        const identifier::Nym& owner,
        opentxs::blockchain::Type chain,
        opentxs::blockchain::crypto::SubaccountType type) const noexcept -> void
    {
    }
};
}  // namespace opentxs::api::client::implementation

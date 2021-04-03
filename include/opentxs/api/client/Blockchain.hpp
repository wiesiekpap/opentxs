// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/BalanceNode.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class BalanceTree;
class HD;
class PaymentCode;
}  // namespace blockchain
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block

class Network;
}  // namespace blockchain

namespace proto
{
class HDPath;
}  // namespace proto

class Contact;
class Identifier;
class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain
{
public:
    using Chain = opentxs::blockchain::Type;
    using Style = blockchain::AddressStyle;
    using DecodedAddress = std::tuple<OTData, Style, std::set<Chain>, bool>;
    using ContactList = std::set<OTIdentifier>;
    using Tx = opentxs::blockchain::block::bitcoin::Transaction;
    using Txid = opentxs::blockchain::block::Txid;
    using TxidHex = std::string;
    using PatternID = opentxs::blockchain::PatternID;

    enum class AccountType : std::uint8_t {
        HD = 0,
    };

    // Throws std::out_of_range for invalid chains
    OPENTXS_EXPORT static Bip44Type Bip44(Chain chain) noexcept(false);

    /// Throws std::runtime_error if type is invalid
    OPENTXS_EXPORT virtual const blockchain::BalanceTree& Account(
        const identifier::Nym& nymID,
        const Chain chain) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual std::set<OTIdentifier> AccountList(
        const identifier::Nym& nymID,
        const Chain chain) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string ActivityDescription(
        const identifier::Nym& nym,
        const Identifier& thread,
        const std::string& threadItemID) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string ActivityDescription(
        const identifier::Nym& nym,
        const Chain chain,
        const Tx& transaction) const noexcept = 0;
    OPENTXS_EXPORT virtual bool AssignContact(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Subchain subchain,
        const Bip32Index index,
        const Identifier& label) const noexcept = 0;
    OPENTXS_EXPORT virtual bool AssignLabel(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Subchain subchain,
        const Bip32Index index,
        const std::string& label) const noexcept = 0;
    OPENTXS_EXPORT virtual bool AssignTransactionMemo(
        const TxidHex& id,
        const std::string& label) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string CalculateAddress(
        const opentxs::blockchain::Type chain,
        const blockchain::AddressStyle format,
        const Data& pubkey) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Confirm(
        const blockchain::Key key,
        const opentxs::blockchain::block::Txid& tx) const noexcept = 0;
    OPENTXS_EXPORT virtual DecodedAddress DecodeAddress(
        const std::string& encoded) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Disable(const Chain type) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Enable(
        const Chain type,
        const std::string& seednode = "") const noexcept = 0;
    OPENTXS_EXPORT virtual std::set<Chain> EnabledChains() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string EncodeAddress(
        const Style style,
        const Chain chain,
        const Data& data) const noexcept = 0;
    /// throws std::out_of_range if chain has not been started
    OPENTXS_EXPORT virtual const opentxs::blockchain::Network& GetChain(
        const Chain type) const noexcept(false) = 0;
    /// Throws std::out_of_range if the specified key does not exist
    OPENTXS_EXPORT virtual const blockchain::BalanceNode::Element& GetKey(
        const blockchain::Key& id) const noexcept(false) = 0;
    /// Throws std::out_of_range if the specified account does not exist
    OPENTXS_EXPORT virtual const blockchain::HD& HDSubaccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual PatternID IndexItem(
        const ReadView bytes) const noexcept = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<const Tx> LoadTransactionBitcoin(
        const Txid& id) const noexcept = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<const Tx> LoadTransactionBitcoin(
        const TxidHex& id) const noexcept = 0;
    OPENTXS_EXPORT virtual ContactList LookupContacts(
        const std::string& address) const noexcept = 0;
    OPENTXS_EXPORT virtual ContactList LookupContacts(
        const Data& pubkeyHash) const noexcept = 0;
    OPENTXS_EXPORT virtual OTIdentifier NewHDSubaccount(
        const identifier::Nym& nymID,
        const BlockchainAccountType standard,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept = 0;
    OPENTXS_EXPORT virtual OTIdentifier NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept = 0;
    OPENTXS_EXPORT virtual const identifier::Nym& Owner(
        const Identifier& accountID) const noexcept = 0;
    OPENTXS_EXPORT virtual const identifier::Nym& Owner(
        const blockchain::Key& key) const noexcept = 0;
    /// Throws std::out_of_range if the specified account does not exist
    OPENTXS_EXPORT virtual const blockchain::PaymentCode& PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual const blockchain::PaymentCode& PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual bool ProcessTransaction(
        const Chain chain,
        const Tx& transaction,
        const PasswordPrompt& reason) const noexcept = 0;
    OPENTXS_EXPORT virtual OTIdentifier RecipientContact(
        const blockchain::Key& key) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Release(
        const blockchain::Key key) const noexcept = 0;
    OPENTXS_EXPORT virtual OTIdentifier SenderContact(
        const blockchain::Key& key) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Start(
        const Chain type,
        const std::string& seednode = "") const noexcept = 0;
    OPENTXS_EXPORT virtual bool StartSyncServer(
        const std::string& syncEndpoint,
        const std::string& publicSyncEndpoint,
        const std::string& updateEndpoint,
        const std::string& publicUpdateEndpoint) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Stop(const Chain type) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Unconfirm(
        const blockchain::Key key,
        const opentxs::blockchain::block::Txid& tx,
        const Time time = Clock::now()) const noexcept = 0;

    OPENTXS_EXPORT virtual ~Blockchain() = default;

protected:
    Blockchain() noexcept = default;

private:
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;
    Blockchain& operator=(Blockchain&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_BLOCKCHAIN_HPP

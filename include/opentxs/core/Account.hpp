// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_ACCOUNT_HPP
#define OPENTXS_CORE_ACCOUNT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>
#include <string>

#include "opentxs/Exclusive.hpp"
#include "opentxs/Shared.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

namespace opentxs
{
namespace api
{
namespace implementation
{
class Wallet;
}  // namespace implementation

namespace server
{
namespace implementation
{
class Wallet;
}  // namespace implementation
}  // namespace server

class Core;
}  // namespace api

namespace identifier
{
class Nym;
class Server;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
class Base;
}  // namespace context
}  // namespace otx

class Account;
class Ledger;
class OTWallet;
class PasswordPrompt;
class Tag;
}  // namespace opentxs

namespace opentxs
{
using ExclusiveAccount = Exclusive<Account>;
using SharedAccount = Shared<Account>;

class OPENTXS_EXPORT Account : public OTTransactionType
{
public:
    // If you add any types to this list, update the list of strings at the
    // top of the .cpp file.
    enum AccountType {
        user,       // used by users
        issuer,     // used by issuers    (these can only go negative.)
        basket,     // issuer acct used by basket currencies (these can only go
                    // negative)
        basketsub,  // used by the server (to store backing reserves for basket
                    // sub-accounts)
        mint,       // used by mints (to store backing reserves for cash)
        voucher,    // used by the server (to store backing reserves for
                    // vouchers)
        stash,  // used by the server (to store backing reserves for stashes,
                // for smart contracts.)
        err_acct
    };

    static auto _GetTypeString(AccountType accountType) -> char const*;

    auto Alias() const -> std::string;
    auto ConsensusHash(
        const otx::context::Base& context,
        Identifier& theOutput,
        const PasswordPrompt& reason) const -> bool;
    auto DisplayStatistics(String& contents) const -> bool override;
    auto GetBalance() const -> Amount;
    auto GetInstrumentDefinitionID() const -> const identifier::UnitDefinition&;
    auto GetStashTransNum() const -> TransactionNumber
    {
        return stashTransNum_;
    }
    auto GetTypeString() const -> char const*
    {
        return _GetTypeString(acctType_);
    }
    auto IsAllowedToGoNegative() const -> bool;
    auto IsInternalServerAcct() const -> bool;
    auto IsOwnedByUser() const -> bool;
    auto IsOwnedByEntity() const -> bool;
    auto IsIssuer() const -> bool;
    // For accounts used by smart contracts, to stash funds while running.
    auto IsStashAcct() const -> bool { return (acctType_ == stash); }
    auto LoadInbox(const identity::Nym& nym) const -> std::unique_ptr<Ledger>;
    auto LoadOutbox(const identity::Nym& nym) const -> std::unique_ptr<Ledger>;
    // Compares the NymID loaded from the account file with whatever Nym the
    // programmer wants to verify.
    auto VerifyOwner(const identity::Nym& candidate) const -> bool;
    auto VerifyOwnerByID(const identifier::Nym& nymId) const -> bool;

    // Debit a certain amount from the account (presumably the same amount is
    // being added somewhere)
    auto Debit(const Amount amount) -> bool;
    // Credit a certain amount from the account (presumably the same amount is
    // being subtracted somewhere)
    auto Credit(const Amount amount) -> bool;
    auto GetInboxHash(Identifier& output) -> bool;
    auto GetOutboxHash(Identifier& output) -> bool;
    auto InitBoxes(const identity::Nym& signer, const PasswordPrompt& reason)
        -> bool;
    // If you pass the identifier in, the inbox hash is recorded there
    auto SaveInbox(Ledger& box) -> bool;
    auto SaveInbox(Ledger& box, Identifier& hash) -> bool;
    // If you pass the identifier in, the outbox hash is recorded there
    auto SaveOutbox(Ledger& box) -> bool;
    auto SaveOutbox(Ledger& box, Identifier& hash) -> bool;
    void SetAlias(const std::string& alias);
    void SetInboxHash(const Identifier& input);
    void SetOutboxHash(const Identifier& input);
    void SetStashTransNum(const TransactionNumber transNum)
    {
        stashTransNum_ = transNum;
    }

    ~Account() override;

private:
    friend OTWallet;
    friend opentxs::api::implementation::Wallet;
    friend opentxs::api::server::implementation::Wallet;

    AccountType acctType_{err_acct};
    // These are all the variables from the account file itself.
    OTUnitID acctInstrumentDefinitionID_;
    OTString balanceDate_;
    OTString balanceAmount_;
    // the Transaction Number of a smart contract running on cron, if this is a
    // stash account.
    TransactionNumber stashTransNum_{0};
    // Default FALSE. When set to true, saves a "DELETED" flag with this Account
    bool markForDeletion_{false};
    // for easy cleanup later when the server is doing some maintenance.
    // Hash of this account's Inbox, so we don't download it more often than
    // necessary.
    OTIdentifier inboxHash_;
    // Hash of this account's Outbox, so we don't download it more often than
    // necessary.
    OTIdentifier outboxHash_;
    std::string alias_;

    static auto GenerateNewAccount(
        const api::Core& api,
        const identifier::Nym& nymID,
        const identifier::Server& notaryID,
        const identity::Nym& serverNym,
        const Identifier& userNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const PasswordPrompt& reason,
        AccountType acctType = user,
        TransactionNumber stashTransNum = 0) -> Account*;
    // Let's say you don't have or know the NymID, and you just want to load
    // the damn thing up. Then call this function. It will set nymID for you.
    static auto LoadExistingAccount(
        const api::Core& api,
        const Identifier& accountId,
        const identifier::Server& notaryID) -> Account*;

    auto SaveContractWallet(Tag& parent) const -> bool override;

    auto create_box(
        std::unique_ptr<Ledger>& box,
        const identity::Nym& signer,
        const ledgerType type,
        const PasswordPrompt& reason) -> bool;
    auto GenerateNewAccount(
        const identity::Nym& server,
        const Identifier& userNymID,
        const identifier::Server& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const PasswordPrompt& reason,
        AccountType acctType = user,
        std::int64_t stashTransNum = 0) -> bool;
    void InitAccount();
    // overriding this so I can set filename automatically inside based on ID.
    auto LoadContract() -> bool override;
    auto LoadContractFromString(const String& theStr) -> bool override;
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;
    void Release() override;
    void Release_Account();
    // generates filename based on accounts path and account ID. Saves to the
    // standard location for an acct.
    auto SaveAccount() -> bool;

    auto save_box(
        Ledger& box,
        Identifier& hash,
        bool (Ledger::*save)(Identifier&),
        void (Account::*set)(const Identifier&)) -> bool;

    void UpdateContents(const PasswordPrompt& reason) override;

    Account(
        const api::Core& api,
        const identifier::Nym& nymID,
        const Identifier& accountId,
        const identifier::Server& notaryID,
        const String& name);
    Account(
        const api::Core& api,
        const identifier::Nym& nymID,
        const Identifier& accountId,
        const identifier::Server& notaryID);
    Account(
        const api::Core& api,
        const identifier::Nym& nymID,
        const identifier::Server& notaryID);
    Account(const api::Core& api);
    Account() = delete;
};
}  // namespace opentxs
#endif

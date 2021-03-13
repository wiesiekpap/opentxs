// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_LEDGER_HPP
#define OPENTXS_CORE_LEDGER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
namespace api
{
namespace implementation
{
class Factory;
}  // namespace implementation

namespace internal
{
struct Core;
}  // namespace internal
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
class Server;
}  // namespace context
}  // namespace otx

class Account;
class Identifier;
class Item;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
// transaction ID is a std::int64_t, assigned by the server. Each transaction
// has one. FIRST the server issues the ID. THEN we create the blank transaction
// object with the ID in it and store it in our inbox. THEN if we want to send a
// transaction, we use the blank to do so. If there is no blank available, we
// message the server and request one.
using mapOfTransactions =
    std::map<TransactionNumber, std::shared_ptr<OTTransaction>>;

// the "inbox" and "outbox" functionality is implemented in this class
class Ledger : public OTTransactionType
{
public:
    ledgerType m_Type;
    // So the server can tell if it just loaded a legacy box or a hashed box.
    // (Legacy boxes stored ALL of the receipts IN the box. No more.)
    bool m_bLoadedLegacyData;

    inline ledgerType GetType() const { return m_Type; }

    OPENTXS_EXPORT bool LoadedLegacyData() const { return m_bLoadedLegacyData; }

    // This function assumes that this is an INBOX.
    // If you don't use an INBOX to call this method, then it will return
    // nullptr immediately. If you DO use an inbox, then it will create a
    // balanceStatement item to go onto your transaction.  (Transactions require
    // balance statements. And when you get the atBalanceStatement reply from
    // the server, KEEP THAT RECEIPT. Well, OT will do that for you.) You only
    // have to keep the latest receipt, unlike systems that don't store balance
    // agreement.  We also store a list of issued transactions, the new balance,
    // and the outbox hash.
    OPENTXS_EXPORT std::unique_ptr<Item> GenerateBalanceStatement(
        std::int64_t lAdjustment,
        const OTTransaction& theOwner,
        const otx::context::Server& context,
        const Account& theAccount,
        Ledger& theOutbox,
        const PasswordPrompt& reason) const;
    OPENTXS_EXPORT std::unique_ptr<Item> GenerateBalanceStatement(
        std::int64_t lAdjustment,
        const OTTransaction& theOwner,
        const otx::context::Server& context,
        const Account& theAccount,
        Ledger& theOutbox,
        const std::set<TransactionNumber>& without,
        const PasswordPrompt& reason) const;

    OPENTXS_EXPORT void ProduceOutboxReport(
        Item& theBalanceItem,
        const PasswordPrompt& reason);

    OPENTXS_EXPORT bool AddTransaction(
        std::shared_ptr<OTTransaction> theTransaction);
    OPENTXS_EXPORT bool RemoveTransaction(
        const TransactionNumber number);  // if false,
                                          // transaction
                                          // wasn't
                                          // found.

    OPENTXS_EXPORT std::set<std::int64_t> GetTransactionNums(
        const std::set<std::int32_t>* pOnlyForIndices = nullptr) const;

    OPENTXS_EXPORT std::shared_ptr<OTTransaction> GetTransaction(
        transactionType theType);
    OPENTXS_EXPORT std::shared_ptr<OTTransaction> GetTransaction(
        const TransactionNumber number) const;
    OPENTXS_EXPORT std::shared_ptr<OTTransaction> GetTransactionByIndex(
        std::int32_t nIndex) const;
    OPENTXS_EXPORT std::shared_ptr<OTTransaction> GetFinalReceipt(
        std::int64_t lReferenceNum);
    OPENTXS_EXPORT std::shared_ptr<OTTransaction> GetTransferReceipt(
        std::int64_t lNumberOfOrigin);
    OPENTXS_EXPORT std::shared_ptr<OTTransaction> GetChequeReceipt(
        std::int64_t lChequeNum);
    OPENTXS_EXPORT std::int32_t GetTransactionIndex(
        const TransactionNumber number);  // if not
                                          // found,
                                          // returns -1
    OPENTXS_EXPORT std::shared_ptr<OTTransaction> GetReplyNotice(
        const std::int64_t& lRequestNum);

    // This calls OTTransactionType::VerifyAccount(), which calls
    // VerifyContractID() as well as VerifySignature().
    //
    // But first, this OTLedger version also loads the box receipts,
    // if doing so is appropriate. (message ledger == not appropriate.)
    //
    // Use this method instead of Contract::VerifyContract, which
    // expects/uses a pubkey from inside the contract in order to verify
    // it.
    //
    OPENTXS_EXPORT bool VerifyAccount(const identity::Nym& theNym) override;
    // For ALL abbreviated transactions, load the actual box receipt for each.
    OPENTXS_EXPORT bool LoadBoxReceipts(
        std::set<std::int64_t>* psetUnloaded = nullptr);  // if psetUnloaded
                                                          // passed
                                                          // in, then use it to
                                                          // return the #s that
                                                          // weren't there.
    OPENTXS_EXPORT bool SaveBoxReceipts();  // For all "full version"
                                            // transactions, save the actual box
                                            // receipt for each.
    // Verifies the abbreviated form exists first, and then loads the
    // full version and compares the two. Returns success / fail.
    //
    OPENTXS_EXPORT bool LoadBoxReceipt(const std::int64_t& lTransactionNum);
    // Saves the Box Receipt separately.
    OPENTXS_EXPORT bool SaveBoxReceipt(const std::int64_t& lTransactionNum);
    // "Deletes" it by adding MARKED_FOR_DELETION to the bottom of the file.
    OPENTXS_EXPORT bool DeleteBoxReceipt(const std::int64_t& lTransactionNum);

    OPENTXS_EXPORT bool LoadInbox();
    OPENTXS_EXPORT bool LoadNymbox();
    OPENTXS_EXPORT bool LoadOutbox();

    // If you pass the identifier in, the hash is recorded there
    OPENTXS_EXPORT bool SaveInbox();
    OPENTXS_EXPORT bool SaveInbox(Identifier& pInboxHash);
    OPENTXS_EXPORT bool SaveNymbox();
    OPENTXS_EXPORT bool SaveNymbox(Identifier& pNymboxHash);
    OPENTXS_EXPORT bool SaveOutbox();
    OPENTXS_EXPORT bool SaveOutbox(Identifier& pOutboxHash);

    OPENTXS_EXPORT bool CalculateHash(Identifier& theOutput) const;
    OPENTXS_EXPORT bool CalculateInboxHash(Identifier& theOutput) const;
    OPENTXS_EXPORT bool CalculateOutboxHash(Identifier& theOutput) const;
    OPENTXS_EXPORT bool CalculateNymboxHash(Identifier& theOutput) const;
    OPENTXS_EXPORT bool SavePaymentInbox();
    OPENTXS_EXPORT bool LoadPaymentInbox();

    OPENTXS_EXPORT bool SaveRecordBox();
    OPENTXS_EXPORT bool LoadRecordBox();

    OPENTXS_EXPORT bool SaveExpiredBox();
    OPENTXS_EXPORT bool LoadExpiredBox();
    OPENTXS_EXPORT bool LoadLedgerFromString(
        const String& theStr);  // Auto-detects
                                // ledger
                                // type.
    OPENTXS_EXPORT bool LoadInboxFromString(const String& strBox);
    OPENTXS_EXPORT bool LoadOutboxFromString(const String& strBox);
    OPENTXS_EXPORT bool LoadNymboxFromString(const String& strBox);
    OPENTXS_EXPORT bool LoadPaymentInboxFromString(const String& strBox);
    OPENTXS_EXPORT bool LoadRecordBoxFromString(const String& strBox);
    OPENTXS_EXPORT bool LoadExpiredBoxFromString(const String& strBox);
    // inline for the top one only.
    inline std::int32_t GetTransactionCount() const
    {
        return static_cast<std::int32_t>(m_mapTransactions.size());
    }
    OPENTXS_EXPORT std::int32_t GetTransactionCountInRefTo(
        std::int64_t lReferenceNum) const;
    OPENTXS_EXPORT std::int64_t GetTotalPendingValue(
        const PasswordPrompt& reason);  // for inbox only, allows you
                                        // to
    // lookup the total value of pending
    // transfers within.
    OPENTXS_EXPORT const mapOfTransactions& GetTransactionMap() const;

    OPENTXS_EXPORT void Release() override;
    OPENTXS_EXPORT void Release_Ledger();

    OPENTXS_EXPORT void ReleaseTransactions();
    // ONLY call this if you need to load a ledger where you don't already know
    // the person's NymID
    // For example, if you need to load someone ELSE's inbox in order to send
    // them a transfer, then
    // you only know their account number, not their user ID. So you call this
    // function to get it
    // loaded up, and the NymID will hopefully be loaded up with the rest of
    // it.
    OPENTXS_EXPORT void InitLedger();

    [[deprecated]] OPENTXS_EXPORT bool GenerateLedger(
        const Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false);
    OPENTXS_EXPORT bool CreateLedger(
        const identifier::Nym& theNymID,
        const Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false);

    OPENTXS_EXPORT static char const* _GetTypeString(ledgerType theType);
    OPENTXS_EXPORT char const* GetTypeString() const
    {
        return _GetTypeString(m_Type);
    }

    OPENTXS_EXPORT ~Ledger() override;

protected:
    bool LoadGeneric(
        ledgerType theType,
        const String& pString = String::Factory());
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;
    bool SaveGeneric(ledgerType theType);
    void UpdateContents(const PasswordPrompt& reason)
        override;  // Before transmission or
                   // serialization, this is where the
                   // ledger saves its contents

private:  // Private prevents erroneous use by other classes.
    friend api::implementation::Factory;

    using ot_super = OTTransactionType;

    mapOfTransactions m_mapTransactions;  // a ledger contains a map of
                                          // transactions.

    std::tuple<bool, std::string, std::string, std::string> make_filename(
        const ledgerType theType);

    bool generate_ledger(
        const identifier::Nym& theNymID,
        const Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile);
    bool save_box(
        const ledgerType type,
        Identifier& hash,
        bool (Ledger::*calc)(Identifier&) const);

    Ledger(const api::internal::Core& api);
    Ledger(
        const api::internal::Core& api,
        const Identifier& theAccountID,
        const identifier::Server& theNotaryID);
    Ledger(
        const api::internal::Core& api,
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const identifier::Server& theNotaryID);
    Ledger() = delete;
};
}  // namespace opentxs
#endif

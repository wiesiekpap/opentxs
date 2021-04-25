// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Base class for OTTrade and OTAgreement.
// OTCron contains lists of these for regular processing.

#ifndef OPENTXS_CORE_CRON_OTCRONITEM_HPP
#define OPENTXS_CORE_CRON_OTCRONITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <chrono>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/OTTrackable.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal

class Wallet;
}  // namespace api

namespace identifier
{
class Server;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
class Client;
class Server;
}  // namespace context
}  // namespace otx

class NumList;
class OTCron;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT OTCronItem : public OTTrackable
{
public:
    virtual originType GetOriginType() const = 0;

    // To force the Nym to close out the closing number on the receipt.
    bool DropFinalReceiptToInbox(
        const identifier::Nym& NYM_ID,
        const Identifier& ACCOUNT_ID,
        const std::int64_t& lNewTransactionNumber,
        const std::int64_t& lClosingNumber,
        const String& strOrigCronItem,
        const originType theOriginType,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory());

    // Notify the Nym that the OPENING number is now closed, so he can remove it
    // from his issued list.
    bool DropFinalReceiptToNymbox(
        const identifier::Nym& NYM_ID,
        const TransactionNumber& lNewTransactionNumber,
        const String& strOrigCronItem,
        const originType theOriginType,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory());
    virtual bool CanRemoveItemFromCron(const otx::context::Client& context);
    virtual void HarvestOpeningNumber(otx::context::Server& context);
    virtual void HarvestClosingNumbers(otx::context::Server& context);
    // pActivator and pRemover are both "SOMETIMES nullptr"
    // I don't default the parameter, because I want to force the programmer to
    // choose.

    // Called in OTCron::AddCronItem.
    void HookActivationOnCron(
        const PasswordPrompt& reason,
        bool bForTheFirstTime = false);  // This calls
                                         // onActivate,
                                         // which is
                                         // virtual.

    // Called in OTCron::RemoveCronItem as well as OTCron::ProcessCron.
    // This calls onFinalReceipt, then onRemovalFromCron. Both are virtual.
    void HookRemovalFromCron(
        const api::Wallet& wallet,
        Nym_p pRemover,
        std::int64_t newTransactionNo,
        const PasswordPrompt& reason);

    inline bool IsFlaggedForRemoval() const { return m_bRemovalFlag; }
    inline void FlagForRemoval() { m_bRemovalFlag = true; }
    inline void SetCronPointer(OTCron& theCron) { m_pCron = &theCron; }

    static std::unique_ptr<OTCronItem> LoadCronReceipt(
        const api::internal::Core& api,
        const TransactionNumber& lTransactionNum);  // Server-side only.
    static std::unique_ptr<OTCronItem> LoadActiveCronReceipt(
        const api::internal::Core& api,
        const TransactionNumber& lTransactionNum,
        const identifier::Server& notaryID);  // Client-side only.
    static bool EraseActiveCronReceipt(
        const api::internal::Core& api,
        const std::string& dataFolder,
        const TransactionNumber& lTransactionNum,
        const identifier::Nym& nymID,
        const identifier::Server& notaryID);  // Client-side only.
    static bool GetActiveCronTransNums(
        const api::internal::Core& api,
        NumList& output,  // Client-side
                          // only.
        const std::string& dataFolder,
        const identifier::Nym& nymID,
        const identifier::Server& notaryID);
    inline void SetCreationDate(const Time CREATION_DATE)
    {
        m_CREATION_DATE = CREATION_DATE;
    }
    inline const Time GetCreationDate() const { return m_CREATION_DATE; }

    bool SetDateRange(
        const Time VALID_FROM = Time{},
        const Time VALID_TO = Time{});
    inline void SetLastProcessDate(const Time THE_DATE)
    {
        m_LAST_PROCESS_DATE = THE_DATE;
    }

    inline const Time GetLastProcessDate() const { return m_LAST_PROCESS_DATE; }

    inline void SetProcessInterval(const std::chrono::seconds interval)
    {
        m_PROCESS_INTERVAL = interval;
    }
    inline const std::chrono::seconds GetProcessInterval() const
    {
        return m_PROCESS_INTERVAL;
    }

    inline OTCron* GetCron() const { return m_pCron; }
    void setServerNym(Nym_p serverNym) { serverNym_ = serverNym; }
    void setNotaryID(const identifier::Server& notaryID);
    // When first adding anything to Cron, a copy needs to be saved in a
    // folder somewhere.
    bool SaveCronReceipt();  // server side only
    bool SaveActiveCronReceipt(const identifier::Nym& theNymID);  // client
                                                                  // side
                                                                  // only

    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    virtual bool ProcessCron(
        const PasswordPrompt& reason);  // OTCron calls this
                                        // regularly, which is my
                                        // chance to expire, etc.
                                        // From OTTrackable
                                        // (parent class of this)
    ~OTCronItem() override;

    void InitCronItem();

    void Release() override;
    void Release_CronItem();
    bool GetCancelerID(identifier::Nym& theOutput) const;
    bool IsCanceled() const { return m_bCanceled; }

    // When canceling a cron item before it
    // has been activated, use this.
    bool CancelBeforeActivation(
        const identity::Nym& theCancelerNym,
        const PasswordPrompt& reason);

    // These are for     std::deque<std::int64_t> m_dequeClosingNumbers;
    // They are numbers used for CLOSING a transaction. (finalReceipt.)
    std::int64_t GetClosingTransactionNoAt(std::uint32_t nIndex) const;
    std::int32_t GetCountClosingNumbers() const;

    void AddClosingTransactionNo(const std::int64_t& lClosingTransactionNo);

    // HIGHER LEVEL ABSTRACTIONS:
    std::int64_t GetOpeningNum() const;
    std::int64_t GetClosingNum() const;
    virtual bool IsValidOpeningNumber(const std::int64_t& lOpeningNum) const;

    virtual std::int64_t GetOpeningNumber(
        const identifier::Nym& theNymID) const;
    virtual std::int64_t GetClosingNumber(const Identifier& theAcctID) const;
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

protected:
    std::deque<std::int64_t> m_dequeClosingNumbers;  // Numbers used for
                                                     // CLOSING a
                                                     // transaction.
                                                     // (finalReceipt.)
    OTNymID m_pCancelerNymID;

    bool m_bCanceled{false};  // This defaults to false. But if someone
                              // cancels it (BEFORE it is ever activated,
                              // just to nip it in the bud and harvest the
                              // numbers, and send the notices, etc) -- then
                              // we set this to true, and we also set the
                              // canceler Nym ID. (So we can see these
                              // values later and know whether it was
                              // canceled before activation, and if so, who
                              // did it.)

    bool m_bRemovalFlag{false};  // Set this to true and the cronitem will
                                 // be removed from Cron on next process.
    // (And its offer will be removed from the Market as well, if
    // appropriate.)
    virtual void onActivate([[maybe_unused]] const PasswordPrompt& reason) {
    }  // called by HookActivationOnCron().

    virtual void onFinalReceipt(
        OTCronItem& theOrigCronItem,
        const std::int64_t& lNewTransactionNumber,
        Nym_p theOriginator,
        Nym_p pRemover,
        const PasswordPrompt& reason) = 0;  // called by
                                            // HookRemovalFromCron().

    virtual void onRemovalFromCron(
        [[maybe_unused]] const PasswordPrompt& reason)
    {
    }  // called by HookRemovalFromCron().
    void ClearClosingNumbers();

    OTCronItem(
        const api::internal::Core& api,
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);
    OTCronItem(
        const api::internal::Core& api,
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const Identifier& ACCT_ID,
        const identifier::Nym& NYM_ID);
    OTCronItem(const api::internal::Core& api);

private:
    using ot_super = OTTrackable;

    OTCron* m_pCron;
    Nym_p serverNym_;
    OTIdentifier notaryID_;
    Time m_CREATION_DATE;      // The date, in seconds, when the CronItem was
                               // authorized.
    Time m_LAST_PROCESS_DATE;  // The last time this item was processed.
    std::chrono::seconds m_PROCESS_INTERVAL;  // How often to Process Cron
                                              // on this item.

    OTCronItem() = delete;
    OTCronItem(const OTCronItem&) = delete;
    OTCronItem(OTCronItem&&) = delete;
    OTCronItem& operator=(const OTCronItem&) = delete;
    OTCronItem& operator=(OTCronItem&&) = delete;
};
}  // namespace opentxs
#endif

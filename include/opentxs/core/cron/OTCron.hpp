// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRON_OTCRON_HPP
#define OPENTXS_CORE_CRON_OTCRON_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <chrono>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Server.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal

namespace server
{
namespace implementation
{
class Factory;
}  // namespace implementation

namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace server
}  // namespace api

namespace identifier
{
class Nym;
class UnitDefinition;
}  // namespace identifier

class Armored;
class Identifier;
class OTCronItem;
class OTMarket;
class PasswordPrompt;

/** mapOfCronItems:      Mapped (uniquely) to transaction number. */
using mapOfCronItems = std::map<std::int64_t, std::shared_ptr<OTCronItem>>;
/** multimapOfCronItems: Mapped to date the item was added to Cron. */
using multimapOfCronItems = std::multimap<Time, std::shared_ptr<OTCronItem>>;
/** Mapped (uniquely) to market ID. */
using mapOfMarkets = std::map<std::string, std::shared_ptr<OTMarket>>;
/** Cron stores a bunch of these on this list, which the server refreshes from
 * time to time. */
using listOfLongNumbers = std::list<std::int64_t>;

/** OTCron has a list of OTCronItems. (Really subclasses of that such as OTTrade
 * and OTAgreement.) */
class OTCron final : public Contract
{
public:
    static std::chrono::milliseconds GetCronMsBetweenProcess()
    {
        return __cron_ms_between_process;
    }
    static void SetCronMsBetweenProcess(std::chrono::milliseconds lMS)
    {
        __cron_ms_between_process = lMS;
    }

    static std::int32_t GetCronRefillAmount() { return __trans_refill_amount; }
    static void SetCronRefillAmount(std::int32_t nAmount)
    {
        __trans_refill_amount = nAmount;
    }
    static std::int32_t GetCronMaxItemsPerNym()
    {
        return __cron_max_items_per_nym;
    }
    static void SetCronMaxItemsPerNym(std::int32_t nMax)
    {
        __cron_max_items_per_nym = nMax;
    }
    inline bool IsActivated() const { return m_bIsActivated; }
    inline bool ActivateCron()
    {
        if (!m_bIsActivated)
            return m_bIsActivated = true;
        else
            return false;
    }
    // RECURRING TRANSACTIONS
    bool AddCronItem(
        std::shared_ptr<OTCronItem> theItem,
        const bool bSaveReceipt,
        const Time tDateAdded);  // Date it was FIRST added to Cron.
    /** if returns false, item wasn't found. */
    bool RemoveCronItem(
        std::int64_t lTransactionNum,
        Nym_p theRemover,
        const PasswordPrompt& reason);
    std::shared_ptr<OTCronItem> GetItemByOfficialNum(
        std::int64_t lTransactionNum);
    std::shared_ptr<OTCronItem> GetItemByValidOpeningNum(
        std::int64_t lOpeningNum);
    mapOfCronItems::iterator FindItemOnMap(std::int64_t lTransactionNum);
    multimapOfCronItems::iterator FindItemOnMultimap(
        std::int64_t lTransactionNum);
    // MARKETS
    bool AddMarket(
        std::shared_ptr<OTMarket> theMarket,
        bool bSaveMarketFile = true);

    std::shared_ptr<OTMarket> GetMarket(const Identifier& MARKET_ID);
    std::shared_ptr<OTMarket> GetOrCreateMarket(
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const std::int64_t& lScale);
    /** This is informational only. It returns OTStorage-type data objects,
     * packed in a string. */
    bool GetMarketList(Armored& ascOutput, std::int32_t& nMarketCount);
    bool GetNym_OfferList(
        Armored& ascOutput,
        const identifier::Nym& NYM_ID,
        std::int32_t& nOfferCount);
    // TRANSACTION NUMBERS
    /**The server starts out putting a bunch of numbers in here so Cron can use
     * them. Then the internal trades and payment plans get numbers from here as
     * needed. Server MUST replenish from time-to-time, or Cron will stop
     * working. Part of using Cron properly is to call ProcessCron() regularly,
     * as well as to call AddTransactionNumber() regularly, in order to keep
     * GetTransactionCount() at some minimum threshold. */
    void AddTransactionNumber(const std::int64_t& lTransactionNum);
    std::int64_t GetNextTransactionNumber();
    /** How many numbers do I currently have on the list? */
    std::int32_t GetTransactionCount() const;
    /** Make sure every time you call this, you check the GetTransactionCount()
     * first and replenish it to whatever your minimum supply is. (The
     * transaction numbers in there must be enough to last for the entire
     * ProcessCronItems() call, and all the trades and payment plans within,
     * since it will not be replenished again at least until the call has
     * finished.) */
    void ProcessCronItems();

    std::chrono::milliseconds computeTimeout();

    inline void SetNotaryID(const identifier::Server& NOTARY_ID)
    {
        m_NOTARY_ID = NOTARY_ID;
    }
    inline const identifier::Server& GetNotaryID() const { return m_NOTARY_ID; }

    inline void SetServerNym(Nym_p pServerNym)
    {
        OT_ASSERT(nullptr != pServerNym);
        m_pServerNym = pServerNym;
    }
    inline Nym_p GetServerNym() const { return m_pServerNym; }

    bool LoadCron();
    bool SaveCron();

    ~OTCron() final;

    void InitCron();

    void Release() final;

    /** return -1 if error, 0 if nothing, and 1 if the node was processed. */
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) final;
    /** Before transmission or serialization, this is where the ledger saves its
     * contents */
    void UpdateContents(const PasswordPrompt& reason) final;

private:
    using ot_super = Contract;

    friend api::server::implementation::Factory;

    // Number of transaction numbers Cron  will grab for itself, when it gets
    // low, before each round.
    static std::int32_t __trans_refill_amount;
    // Number of milliseconds (ideally) between each "Cron Process" event.
    static std::chrono::milliseconds __cron_ms_between_process;
    // Int. The maximum number of cron items any given Nym can have
    // active at the same time.
    static std::int32_t __cron_max_items_per_nym;
    static Time last_executed_;

    // A list of all valid markets.
    mapOfMarkets m_mapMarkets;
    // Cron Items are found on both lists.
    mapOfCronItems m_mapCronItems;
    multimapOfCronItems m_multimapCronItems;
    // Always store this in any object that's associated with a specific server.
    OTServerID m_NOTARY_ID;
    // I can't put receipts in people's inboxes without a supply of these.
    listOfLongNumbers m_listTransactionNumbers;
    // I don't want to start Cron processing until everything else is all loaded
    //  up and ready to go.
    bool m_bIsActivated{false};
    // I'll need this for later.
    Nym_p m_pServerNym{nullptr};

    explicit OTCron(const api::internal::Core& server);

    OTCron() = delete;
};
}  // namespace opentxs
#endif

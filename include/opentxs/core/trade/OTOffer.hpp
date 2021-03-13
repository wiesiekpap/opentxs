// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_TRADE_OTOFFER_HPP
#define OPENTXS_CORE_TRADE_OTOFFER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "opentxs/Types.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Instrument.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

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
class Server;
}  // namespace identifier

class OTTrade;
class PasswordPrompt;

// Each instance of OTOffer represents a Bid or Ask. (A Market has a list of bid
// offers and a list of ask offers.)

/*
 OTOffer

 Offer MUST STORE:

 X 1) Transaction ID (MUST be linked to a trade, so it can expire, and so it can
 be paid for.)
 X 2) ASSET TYPE ID of whatever I’m trying to BUY or SELL. (Is this the Gold
 market?)
 X 7) CURRENCY TYPE ID of whatever I’m trying to buy or sell it WITH. (Is it
 dollars? Euro? Yen?)
 X 8) BUY OR SELL? (BOOL)
 X 9) Bid/Ask price (limit / per minimum increment.)

 X 4) Total number of assets available for sale or purchase. (4 ounces of gold?
 12 ounces of gold?)
 X 5) Number of assets already traded, against the above total.
 X 6) Minimum increment for sale or purchase (if matches “total number of assets
 for sale”, effectively becomes a FILL OR KILL order. MUST be 1 or greater.
 CANNOT be zero.)
*/
class OTOffer : public Instrument
{
public:
    OPENTXS_EXPORT bool MakeOffer(
        bool bBuyingOrSelling,            // True == SELLING, False == BUYING
        const std::int64_t& lPriceLimit,  // Per Scale...
        const std::int64_t& lTotalAssetsOffer,  // Total assets available for
                                                // sale or
                                                // purchase.
        const std::int64_t& lMinimumIncrement,  // The minimum increment that
                                                // must be
        // bought or sold for each transaction
        const std::int64_t& lTransactionNum,  // The transaction number
                                              // authorizing
                                              // this trade.
        const Time VALID_FROM = {},           // defaults to RIGHT NOW
        const Time VALID_TO = {});            // defaults to 24 hours (a
                                              // "Day Order")
    inline void IncrementFinishedSoFar(const std::int64_t& lFinishedSoFar)
    {
        m_lFinishedSoFar += lFinishedSoFar;
    }

    inline std::int64_t GetAmountAvailable() const
    {
        return GetTotalAssetsOnOffer() - GetFinishedSoFar();
    }
    inline const std::int64_t& GetTransactionNum() const
    {
        return m_lTransactionNum;
    }

    inline const std::int64_t& GetPriceLimit() const { return m_lPriceLimit; }
    inline const std::int64_t& GetTotalAssetsOnOffer() const
    {
        return m_lTotalAssetsOffer;
    }
    inline const std::int64_t& GetFinishedSoFar() const
    {
        return m_lFinishedSoFar;
    }
    inline const std::int64_t& GetMinimumIncrement()
    {
        if (m_lMinimumIncrement < 1) m_lMinimumIncrement = 1;
        return m_lMinimumIncrement;
    }
    inline const std::int64_t& GetScale() const { return m_lScale; }

    inline const Identifier& GetCurrencyID() const
    {
        return m_CURRENCY_TYPE_ID;
    }
    inline void SetCurrencyID(const identifier::UnitDefinition& CURRENCY_ID)
    {
        m_CURRENCY_TYPE_ID = CURRENCY_ID;
    }

    // Buying or selling?
    inline bool IsBid() { return !m_bSelling; }
    inline bool IsAsk() { return m_bSelling; }

    bool IsMarketOrder() const;
    bool IsLimitOrder() const;

    // Stores a pointer to theTrade for later use. (Not responsible to clean up,
    // just convenient.)
    inline OTTrade* GetTrade() { return m_pTrade; }
    inline void SetTrade(const OTTrade& theTrade)
    {
        m_pTrade = &(const_cast<OTTrade&>(theTrade));
    }
    // Note: m_tDateAddedToMarket is not saved in the Offer Contract, but
    // OTMarket sets/saves/loads it.
    //
    OPENTXS_EXPORT Time GetDateAddedToMarket() const;  // Used in
                                                       // OTMarket::GetOfferList
                                                       // and GetNymOfferList.
    OPENTXS_EXPORT void SetDateAddedToMarket(
        const Time tDate);  // Used in OTCron when
                            // adding/loading
                            // offers.
    // Overridden from Contract.
    void GetIdentifier(Identifier& theIdentifier) const override;

    void InitOffer();

    void Release() override;
    void Release_Offer();

    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    void UpdateContents(const PasswordPrompt& reason)
        override;  // Before transmission or
                   // serialization, this is where the
                   // ledger saves its contents

    OPENTXS_EXPORT ~OTOffer() override;

protected:
    // If this offer is actually connected to a trade, it will have a pointer.
    OTTrade* m_pTrade{nullptr};
    // GOLD (Asset) is trading for DOLLARS (Currency).
    OTIdentifier m_CURRENCY_TYPE_ID;
    bool m_bSelling{false};  // true = ask. false = bid.
    // If a bid, this is the most I will pay. If an ask, this is the least I
    // will sell for. My limit.
    // (Normally the price I get is whatever is the best one on the market right
    // now.)
    std::int64_t m_lPriceLimit{0};  // Denominated in CURRENCY TYPE, and priced
                                    // per SCALE. 1oz market price limit might
                                    // be 1,300
    // 100oz market price limit might be 130,000 (or 127,987 or whatever)

    std::int64_t m_lTransactionNum{0};    // Matches to an OTTrade stored in
                                          // OTCron.
    std::int64_t m_lTotalAssetsOffer{0};  // Total amount of ASSET TYPE trying
                                          // to BUY or SELL, this trade.
    std::int64_t m_lFinishedSoFar{0};     // Number of ASSETs bought or sold
                                          // already against the above total.

    std::int64_t m_lScale{0};  // 1oz market? 100oz market? 10,000oz market?
                               // This determines size and granularity.
    std::int64_t m_lMinimumIncrement{0};  // Each sale or purchase against the
                                          // above total must be in minimum
                                          // increments.
    // Minimum Increment must be evenly divisible by m_lScale.
    // (This effectively becomes a "FILL OR KILL" order if set to the same value
    // as m_lTotalAssetsOffer. Also, MUST be 1
    // or great. CANNOT be zero. Enforce this at class level. You cannot sell
    // something in minimum increments of 0.)
    inline void SetTransactionNum(const std::int64_t& lTransactionNum)
    {
        m_lTransactionNum = lTransactionNum;
    }
    inline void SetPriceLimit(const std::int64_t& lPriceLimit)
    {
        m_lPriceLimit = lPriceLimit;
    }
    inline void SetTotalAssetsOnOffer(const std::int64_t& lTotalAssets)
    {
        m_lTotalAssetsOffer = lTotalAssets;
    }
    inline void SetFinishedSoFar(const std::int64_t& lFinishedSoFar)
    {
        m_lFinishedSoFar = lFinishedSoFar;
    }
    inline void SetMinimumIncrement(const std::int64_t& lMinIncrement)
    {
        m_lMinimumIncrement = lMinIncrement;
        if (m_lMinimumIncrement < 1) m_lMinimumIncrement = 1;
    }
    inline void SetScale(const std::int64_t& lScale)
    {
        m_lScale = lScale;
        if (m_lScale < 1) m_lScale = 1;
    }

private:
    friend api::implementation::Factory;

    using ot_super = Instrument;

    Time m_tDateAddedToMarket;

    bool isPowerOfTen(const std::int64_t& x);

    OTOffer(const api::internal::Core& core);  // The constructor contains
                                               // the 3 variables needed to
                                               // identify any market.
    OTOffer(
        const api::internal::Core& core,
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const std::int64_t& MARKET_SCALE);
    OTOffer(const OTOffer&) = delete;
    OTOffer(OTOffer&&) = delete;
    OTOffer& operator=(const OTOffer&) = delete;
    OTOffer& operator=(OTOffer&&) = delete;

    OTOffer() = delete;
};
}  // namespace opentxs
#endif

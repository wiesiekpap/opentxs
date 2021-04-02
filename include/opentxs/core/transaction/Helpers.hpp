// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_TRANSACTION_HELPERS_HPP
#define OPENTXS_CORE_TRANSACTION_HELPERS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/OTTransaction.hpp"

namespace opentxs
{
namespace api
{
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

class Identifier;
class Ledger;
class NumList;
class OTTransaction;
class String;

OPENTXS_EXPORT const char* GetTransactionTypeString(
    int transactionTypeIndex);  // enum transactionType
OPENTXS_EXPORT const char* GetOriginTypeToString(
    int originTypeIndex);  // enum
                           // originType

OPENTXS_EXPORT std::int32_t LoadAbbreviatedRecord(
    irr::io::IrrXMLReader*& xml,
    std::int64_t& lNumberOfOrigin,
    originType& theOriginType,
    std::int64_t& lTransactionNum,
    std::int64_t& lInRefTo,
    std::int64_t& lInRefDisplay,
    Time& the_DATE_SIGNED,
    transactionType& theType,
    String& strHash,
    std::int64_t& lAdjustment,
    std::int64_t& lDisplayValue,
    std::int64_t& lClosingNum,
    std::int64_t& lRequestNum,
    bool& bReplyTransSuccess,
    NumList* pNumList = nullptr);

OPENTXS_EXPORT bool VerifyBoxReceiptExists(
    const api::internal::Core& api,
    const std::string& dataFolder,
    const identifier::Server& NOTARY_ID,
    const identifier::Nym& NYM_ID,
    const Identifier& ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox) then
    // pass NYM_ID in this field also.
    std::int32_t nBoxType,  // 0/nymbox, 1/inbox, 2/outbox
    const std::int64_t& lTransactionNum);

OPENTXS_EXPORT std::unique_ptr<OTTransaction> LoadBoxReceipt(
    const api::internal::Core& api,
    OTTransaction& theAbbrev,
    Ledger& theLedger);

OPENTXS_EXPORT std::unique_ptr<OTTransaction> LoadBoxReceipt(
    const api::internal::Core& api,
    OTTransaction& theAbbrev,
    std::int64_t lLedgerType);

OPENTXS_EXPORT bool SetupBoxReceiptFilename(
    const api::internal::Core& api,
    std::int64_t lLedgerType,
    OTTransaction& theTransaction,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename);

OPENTXS_EXPORT bool SetupBoxReceiptFilename(
    const api::internal::Core& api,
    Ledger& theLedger,
    OTTransaction& theTransaction,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename);

OPENTXS_EXPORT bool SetupBoxReceiptFilename(
    const api::internal::Core& api,
    std::int64_t lLedgerType,
    const String& strUserOrAcctID,
    const String& strNotaryID,
    const std::int64_t& lTransactionNum,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename);
}  // namespace opentxs
#endif

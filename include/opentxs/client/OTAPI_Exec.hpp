// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_OTAPI_EXEC_HPP
#define OPENTXS_CLIENT_OTAPI_EXEC_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <mutex>
#include <set>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Activity;
class Contacts;

namespace implementation
{
class Manager;
}  // namespace implementation
}  // namespace client

namespace network
{
class ZMQ;
}  // namespace network

class Core;
}  // namespace api

class OT_API;

class OPENTXS_EXPORT OTAPI_Exec : Lockable
{
public:
    /**

    PROPOSE PAYMENT PLAN --- Returns the payment plan in string form.

    (Called by Merchant.)

    PARAMETER NOTES:
    -- Payment Plan Delay, and Payment Plan Period, both default to 30 days (if
    you pass 0.)

    -- Payment Plan Length, and Payment Plan Max Payments, both default to 0,
    which means
    no maximum length and no maximum number of payments.

    -----------------------------------------------------------------
    FYI, the payment plan creation process (finally) is:

    1) Payment plan is written, and signed, by the recipient. (This function:
    ProposePaymentPlan)
    2) He sends it to the sender, who signs it and submits it.
    (ConfirmPaymentPlan and depositPaymentPlan)
    3) The server loads the recipient nym to verify the transaction
    number. The sender also had to burn a transaction number (to
    submit it) so now, both have verified trns#s in this way.

    ----------------------------------------------------------------------------------------

    FYI, here are all the OT library calls that are performed by this single API
    call:

    OTPaymentPlan * pPlan = new OTPaymentPlan(pAccount->GetRealNotaryID(),
    pAccount->GetInstrumentDefinitionID(),
    pAccount->GetRealAccountID(),    pAccount->GetNymID(),
    RECIPIENT_ACCT_ID, RECIPIENT_NYM_ID);

    ----------------------------------------------------------------------------------------
    From OTAgreement: (This must be called first, before the other two methods
    below can be called.)

    bool    OTAgreement::SetProposal(const identity::Nym& MERCHANT_NYM, const
    OTString& strConsideration,
    const time64_t& VALID_FROM=0, const time64_t& VALID_TO=0);

    ----------------------------------------------------------------------------------------
    (Optional initial payment):
    bool    OTPaymentPlan::SetInitialPayment(const std::int64_t& lAmount,
    time64_t
    tTimeUntilInitialPayment=0); // default: now.
    ----------------------------------------------------------------------------------------

    These two (above and below) can be called independent of each other. You can
    have an initial payment, AND/OR a payment plan.

    ----------------------------------------------------------------------------------------
    (Optional regular payments):
    bool    OTPaymentPlan::SetPaymentPlan(const std::int64_t& lPaymentAmount,
    time64_t tTimeUntilPlanStart  =OT_TIME_MONTH_IN_SECONDS, // Default: 1st
    payment in 30 days
    time64_t tBetweenPayments     =OT_TIME_MONTH_IN_SECONDS, // Default: 30
    days.
    time64_t tPlanLength=0, std::int32_t nMaxPayments=0);
    ----------------------------------------------------------------------------------------
    */
    auto ProposePaymentPlan(
        const std::string& NOTARY_ID,
        const Time& VALID_FROM,  // Default (0 or nullptr) == current time
                                 // measured in seconds since Jan 1970.
        const Time& VALID_TO,    // Default (0 or nullptr) == no expiry /
                                 // cancel
                                 // anytime. Otherwise this is ADDED to
                                 // VALID_FROM (it's a length.)
        const std::string& SENDER_ACCT_ID,  // Mandatory parameters.
        const std::string& SENDER_NYM_ID,   // Both sender and recipient must
                                            // sign before submitting.
        const std::string& PLAN_CONSIDERATION,  // Like a memo.
        const std::string& RECIPIENT_ACCT_ID,   // NOT optional.
        const std::string& RECIPIENT_NYM_ID,  // Both sender and recipient must
                                              // sign before submitting.
        const std::int64_t& INITIAL_PAYMENT_AMOUNT,  // zero or nullptr == no
                                                     // initial
                                                     // payment.
        const std::chrono::seconds& INITIAL_PAYMENT_DELAY,  // seconds from
                                                            // creation date.
                                                            // Default is zero
                                                            // or nullptr.
        const std::int64_t& PAYMENT_PLAN_AMOUNT,  // Zero or nullptr == no
                                                  // regular
                                                  // payments.
        const std::chrono::seconds& PAYMENT_PLAN_DELAY,  // No. of seconds from
                                                         // creation
        // date. Default is zero or nullptr.
        // (Causing 30 days.)
        const std::chrono::seconds& PAYMENT_PLAN_PERIOD,  // No. of seconds
                                                          // between payments.
                                                          // Default is zero or
                                                          // nullptr. (Causing
                                                          // 30 days.)
        const std::chrono::seconds& PAYMENT_PLAN_LENGTH,  // In seconds.
                                                          // Defaults to 0 or
                                                          // nullptr (no maximum
                                                          // length.)
        const std::int32_t& PAYMENT_PLAN_MAX_PAYMENTS  // integer. Defaults to 0
                                                       // or
        // nullptr (no maximum payments.)
    ) const -> std::string;

    // The above version has too many arguments for boost::function apparently
    // (for Chaiscript.)
    // So this is a version of it that compresses those into a fewer number of
    // arguments.
    // (Then it expands them and calls the above version.)
    // See above function for more details on parameters.
    // Basically this version has ALL the same parameters, but it stuffs two or
    // three at a time into
    // a single parameter, as a comma-separated list in string form.
    //
    auto EasyProposePlan(
        const std::string& NOTARY_ID,
        const std::string& DATE_RANGE,  // "from,to"  Default 'from' (0 or "")
                                        // ==
                                        // NOW, and default 'to' (0 or "") == no
                                        // expiry / cancel anytime
        const std::string& SENDER_ACCT_ID,  // Mandatory parameters.
        const std::string& SENDER_NYM_ID,   // Both sender and recipient must
                                            // sign before submitting.
        const std::string& PLAN_CONSIDERATION,  // Like a memo.
        const std::string& RECIPIENT_ACCT_ID,   // NOT optional.
        const std::string& RECIPIENT_NYM_ID,  // Both sender and recipient must
                                              // sign before submitting.
        const std::string& INITIAL_PAYMENT,  // "amount,delay"  Default 'amount'
                                             // (0 or "") == no initial payment.
        // Default 'delay' (0 or nullptr) is
        // seconds from creation date.
        const std::string& PAYMENT_PLAN,  // "amount,delay,period" 'amount' is a
                                          // recurring payment. 'delay' and
        // 'period' cause 30 days if you pass 0
        // or "".
        const std::string& PLAN_EXPIRY  // "length,number" 'length' is maximum
                                        // lifetime in seconds. 'number' is
        // maximum number of payments in seconds.
        // 0 or "" is unlimited (for both.)
    ) const -> std::string;

    // Called by Customer. Pass in the plan obtained in the above call.
    //
    auto ConfirmPaymentPlan(
        const std::string& NOTARY_ID,
        const std::string& SENDER_NYM_ID,
        const std::string& SENDER_ACCT_ID,
        const std::string& RECIPIENT_NYM_ID,
        const std::string& PAYMENT_PLAN) const -> std::string;

    // SMART CONTRACTS

    // RETURNS: the Smart Contract itself. (Or nullptr.)
    //
    auto Create_SmartContract(
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const Time& VALID_FROM,            // Default (0 or nullptr) == NOW
        const Time& VALID_TO,  // Default (0 or nullptr) == no expiry /
                               // cancel anytime
        bool SPECIFY_ASSETS,  // Asset type IDs must be provided for every named
                              // account.
        bool SPECIFY_PARTIES  // Nym IDs must be provided for every party.
    ) const -> std::string;

    auto SmartContract_SetDates(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // dates changed on it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const Time& VALID_FROM,            // Default (0 or nullptr) == NOW
        const Time& VALID_TO  // Default (0 or nullptr) == no expiry /
                              // cancel
                              // anytime
    ) const -> std::string;

    auto Smart_ArePartiesSpecified(const std::string& THE_CONTRACT) const
        -> bool;
    auto Smart_AreAssetTypesSpecified(const std::string& THE_CONTRACT) const
        -> bool;

    //
    // todo: Someday add a parameter here BYLAW_LANGUAGE so that people can use
    // custom languages in their scripts. For now I have a default language, so
    // I'll just make that the default. (There's only one language right now
    // anyway.)
    //
    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddBylaw(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // bylaw added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME  // The Bylaw's NAME as referenced in the
                                       // smart contract. (And the scripts...)
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveBylaw(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // bylaw removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME  // The Bylaw's NAME as referenced in the
                                       // smart contract. (And the scripts...)
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddClause(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // clause added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& CLAUSE_NAME,    // The Clause's name as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
        const std::string& SOURCE_CODE     // The actual source code for the
                                           // clause.
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_UpdateClause(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // clause updated on it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& CLAUSE_NAME,    // The Clause's name as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
        const std::string& SOURCE_CODE     // The actual source code for the
                                           // clause.
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveClause(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // clause removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& CLAUSE_NAME     // The Clause's name as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddVariable(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // variable added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& VAR_NAME,    // The Variable's name as referenced in
                                        // the
                                        // smart contract. (And the scripts...)
        const std::string& VAR_ACCESS,  // "constant", "persistent", or
                                        // "important".
        const std::string& VAR_TYPE,    // "string", "std::int64_t", or "bool"
        const std::string& VAR_VALUE    // Contains a string. If type is
                                        // std::int64_t,
        // atol() will be used to convert value to
        // a std::int64_t. If type is bool, the strings
        // "true" or "false" are expected here in
        // order to convert to a bool.
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveVariable(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // variable removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& VAR_NAME  // The Variable's name as referenced in the
                                     // smart contract. (And the scripts...)
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddCallback(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // callback added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& CALLBACK_NAME,  // The Callback's name as referenced
                                           // in the smart contract. (And the
                                           // scripts...)
        const std::string& CLAUSE_NAME     // The actual clause that will be
                                           // triggered by the callback. (Must
                                           // exist.)
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveCallback(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // callback removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& CALLBACK_NAME   // The Callback's name as referenced
                                           // in the smart contract. (And the
                                           // scripts...)
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddHook(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // hook
                                           // added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& HOOK_NAME,   // The Hook's name as referenced in the
                                        // smart contract. (And the scripts...)
        const std::string& CLAUSE_NAME  // The actual clause that will be
                                        // triggered by the hook. (You can call
        // this multiple times, and have multiple
        // clauses trigger on the same hook.)
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveHook(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // hook
                                           // removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& HOOK_NAME,   // The Hook's name as referenced in the
                                        // smart contract. (And the scripts...)
        const std::string& CLAUSE_NAME  // The actual clause that will be
                                        // triggered by the hook. (You can call
        // this multiple times, and have multiple
        // clauses trigger on the same hook.)
    ) const -> std::string;

    // RETURNS: Updated version of THE_CONTRACT. (Or nullptr.)
    auto SmartContract_AddParty(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // party added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& PARTY_NYM_ID,  // Required when the smart contract is
                                          // configured to require parties to be
                                          // specified. Otherwise must be empty.
        const std::string& PARTY_NAME,  // The Party's NAME as referenced in the
                                        // smart contract. (And the scripts...)
        const std::string& AGENT_NAME   // An AGENT will be added by default for
                                        // this party. Need Agent NAME.
    ) const -> std::string;

    // RETURNS: Updated version of THE_CONTRACT. (Or nullptr.)
    auto SmartContract_RemoveParty(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // party removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& PARTY_NAME  // The Party's NAME as referenced in the
                                       // smart contract. (And the scripts...)
    ) const -> std::string;

    // (FYI, that is basically the only option, until I code Entities and Roles.
    // Until then, a party can ONLY be
    // a Nym, with himself as the agent representing that same party. Nym ID is
    // supplied on ConfirmParty() below.)

    // Used when creating a theoretical smart contract (that could be used over
    // and over again with different parties.)
    //
    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddAccount(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // account added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& PARTY_NAME,  // The Party's NAME as referenced in the
                                        // smart contract. (And the scripts...)
        const std::string& ACCT_NAME,   // The Account's name as referenced in
                                        // the
                                        // smart contract
        const std::string& INSTRUMENT_DEFINITION_ID  // Instrument Definition ID
                                                     // for the
                                                     // Account.
    ) const -> std::string;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveAccount(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // account removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& PARTY_NAME,  // The Party's NAME as referenced in the
                                        // smart contract. (And the scripts...)
        const std::string& ACCT_NAME  // The Account's name as referenced in the
                                      // smart contract
    ) const -> std::string;

    /** This function returns the count of how many trans#s a Nym needs in order
    to confirm as
    // a specific agent for a contract. (An opening number is needed for every
    party of which
    // agent is the authorizing agent, plus a closing number for every acct of
    which agent is the
    // authorized agent.)
    */
    auto SmartContract_CountNumsNeeded(
        const std::string& THE_CONTRACT,  // The smart contract, about to be
                                          // queried by this function.
        const std::string& AGENT_NAME) const -> std::int32_t;

    /** ----------------------------------------
    // Used when taking a theoretical smart contract, and setting it up to use
    specific Nyms and accounts. This function sets the ACCT ID for the acct
    specified by party name and acct name.
    // Returns the updated smart contract (or nullptr.)
    */
    auto SmartContract_ConfirmAccount(
        const std::string& THE_CONTRACT,   // The smart contract, about to be
                                           // changed by this function.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& PARTY_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& ACCT_NAME,      // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& AGENT_NAME,     // The agent name for this asset
                                           // account.
        const std::string& ACCT_ID         // AcctID for the asset account. (For
                                           // acct_name).
    ) const -> std::string;

    /** ----------------------------------------
    // Called by each Party. Pass in the smart contract obtained in the above
    call.
    // Call SmartContract_ConfirmAccount() first, as much as you need to.
    // Returns the updated smart contract (or nullptr.)
    */
    auto SmartContract_ConfirmParty(
        const std::string& THE_CONTRACT,  // The smart contract, about to be
                                          // changed by this function.
        const std::string& PARTY_NAME,    // Should already be on the contract.
                                          // This way we can find it.
        const std::string& NYM_ID,  // Nym ID for the party, the actual owner,
        const std::string& NOTARY_ID) const -> std::string;
    // ===> AS WELL AS for the default AGENT of that party.

    /* ----------------------------------------
    Various informational functions for the Smart Contracts.
    */

    auto Smart_AreAllPartiesConfirmed(const std::string& THE_CONTRACT) const
        -> bool;  // true or false?
    auto Smart_GetBylawCount(const std::string& THE_CONTRACT) const
        -> std::int32_t;
    auto Smart_GetBylawByIndex(
        const std::string& THE_CONTRACT,
        const std::int32_t& nIndex) const
        -> std::string;  // returns the name of the bylaw.
    auto Bylaw_GetLanguage(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME) const -> std::string;
    auto Bylaw_GetClauseCount(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME) const -> std::int32_t;
    auto Clause_GetNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::int32_t& nIndex) const
        -> std::string;  // returns the name of the clause.
    auto Clause_GetContents(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& CLAUSE_NAME) const
        -> std::string;  // returns the contents of the
                         // clause.
    auto Bylaw_GetVariableCount(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME) const -> std::int32_t;
    auto Variable_GetNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::int32_t& nIndex) const
        -> std::string;  // returns the name of the variable.
    auto Variable_GetType(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& VARIABLE_NAME) const
        -> std::string;  // returns the type of the
                         // variable.
    auto Variable_GetAccess(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& VARIABLE_NAME) const
        -> std::string;  // returns the access level of
                         // the
                         // variable.
    auto Variable_GetContents(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& VARIABLE_NAME) const
        -> std::string;  // returns the contents of the
                         // variable.
    auto Bylaw_GetHookCount(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME) const -> std::int32_t;
    auto Hook_GetNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::int32_t& nIndex) const
        -> std::string;  // returns the name of the hook.
    auto Hook_GetClauseCount(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& HOOK_NAME) const
        -> std::int32_t;  // for iterating clauses on a
                          // hook.
    auto Hook_GetClauseAtIndex(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& HOOK_NAME,
        const std::int32_t& nIndex) const -> std::string;
    auto Bylaw_GetCallbackCount(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME) const -> std::int32_t;
    auto Callback_GetNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::int32_t& nIndex) const
        -> std::string;  // returns the name of the callback.
    auto Callback_GetClause(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& CALLBACK_NAME) const
        -> std::string;  // returns name of clause
                         // attached to
                         // callback.
    auto Smart_GetPartyCount(const std::string& THE_CONTRACT) const
        -> std::int32_t;
    auto Smart_GetPartyByIndex(
        const std::string& THE_CONTRACT,
        const std::int32_t& nIndex) const
        -> std::string;  // returns the name of the party.
    auto Smart_IsPartyConfirmed(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME) const -> bool;  // true or false?
    auto Party_GetID(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME) const
        -> std::string;  // returns either NymID or Entity
                         // ID.
    auto Party_GetAcctCount(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME) const -> std::int32_t;
    auto Party_GetAcctNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::int32_t& nIndex) const
        -> std::string;  // returns the name of the clause.
    auto Party_GetAcctID(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::string& ACCT_NAME) const
        -> std::string;  // returns account ID for a given
                         // acct
                         // name.
    auto Party_GetAcctInstrumentDefinitionID(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::string& ACCT_NAME) const
        -> std::string;  // returns instrument definition
                         // ID
                         // for a
                         // given acct
                         // name.
    auto Party_GetAcctAgentName(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::string& ACCT_NAME) const
        -> std::string;  // returns agent name authorized
                         // to
    // administer a given named acct. (If
    // it's set...)
    auto Party_GetAgentCount(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME) const -> std::int32_t;
    auto Party_GetAgentNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::int32_t& nIndex) const
        -> std::string;  // returns the name of the agent.
    auto Party_GetAgentID(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::string& AGENT_NAME) const
        -> std::string;  // returns ID of the agent. (If
                         // there is
                         // one...)

    /** --------------------------------------------------------------
    // IS BASKET CURRENCY ?
    //
    // Tells you whether or not a given instrument definition is actually a
    basket
    currency.
    */
    auto IsBasketCurrency(const std::string& INSTRUMENT_DEFINITION_ID) const
        -> bool;  // returns OT_BOOL
                  // (OT_TRUE or
                  // OT_FALSE aka 1
                  // or 0.)

    /** --------------------------------------------------------------------
    // Get Basket Count (of backing instrument definitions.)
    //
    // Returns the number of instrument definitions that make up this basket.
    // (Or zero.)
    */
    auto Basket_GetMemberCount(
        const std::string& BASKET_INSTRUMENT_DEFINITION_ID) const
        -> std::int32_t;

    /** --------------------------------------------------------------------
    // Get Asset Type of a basket's member currency, by index.
    //
    // (Returns a string containing Instrument Definition ID, or nullptr).
    */
    auto Basket_GetMemberType(
        const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
        const std::int32_t& nIndex) const -> std::string;

    /** ----------------------------------------------------
    // GET BASKET MINIMUM TRANSFER AMOUNT
    //
    // Returns a std::int64_t containing the minimum transfer
    // amount for the entire basket.
    //
    // FOR EXAMPLE:
    // If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
    // then the minimum transfer amount for the basket is 10. This function
    // would return a string containing "10", in that example.
    */
    auto Basket_GetMinimumTransferAmount(
        const std::string& BASKET_INSTRUMENT_DEFINITION_ID) const
        -> std::int64_t;

    /** ----------------------------------------------------
    // GET BASKET MEMBER's MINIMUM TRANSFER AMOUNT
    //
    // Returns a std::int64_t containing the minimum transfer
    // amount for one of the member currencies in the basket.
    //
    // FOR EXAMPLE:
    // If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
    // then the minimum transfer amount for the member currency at index
    // 0 is 2, the minimum transfer amount for the member currency at
    // index 1 is 5, and the minimum transfer amount for the member
    // currency at index 2 is 8.
    */
    auto Basket_GetMemberMinimumTransferAmount(
        const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
        const std::int32_t& nIndex) const -> std::int64_t;

    /** ----------------------------------------------------
    // GENERATE BASKET CREATION REQUEST
    //
    // (returns the basket in string form.)
    //
    // Call AddBasketCreationItem multiple times to add
    // the various currencies to the basket, and then call
    // issueBasket to send the request to the server.
    */
    auto GenerateBasketCreation(
        const std::string& nymID,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight,
        const VersionNumber version = contract::Unit::DefaultVersion) const
        -> std::string;

    /** ----------------------------------------------------
    // ADD BASKET CREATION ITEM
    //
    // (returns the updated basket in string form.)
    //
    // Call GenerateBasketCreation first (above), then
    // call this function multiple times to add the various
    // currencies to the basket, and then call issueBasket
    // to send the request to the server.
    */
    auto AddBasketCreationItem(
        const std::string& basketTemplate,
        const std::string& currencyID,
        const std::uint64_t& weight) const -> std::string;

    /** ----------------------------------------------------
    // GENERATE BASKET EXCHANGE REQUEST
    //
    // (Returns the new basket exchange request in string form.)
    //
    // Call this function first. Then call AddBasketExchangeItem
    // multiple times, and then finally call exchangeBasket to
    // send the request to the server.
    */
    auto GenerateBasketExchange(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
        const std::string& BASKET_ASSET_ACCT_ID,
        const std::int32_t& TRANSFER_MULTIPLE) const -> std::string;

    //! 1    2    3
    //! 5=2,3,4 OR 10=4,6,8 OR 15=6,9,12 Etc. (The MULTIPLE.)

    /** ----------------------------------------------------
    // ADD BASKET EXCHANGE ITEM
    //
    // Returns the updated basket exchange request in string form.
    // (Or nullptr.)
    //
    // Call the above function first. Then call this one multiple
    // times, and then finally call exchangeBasket to send
    // the request to the server.
    */
    auto AddBasketExchangeItem(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& THE_BASKET,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& ASSET_ACCT_ID) const -> std::string;

    /** Import a BIP39 seed into the wallet.
     *
     *  The imported seed will be set to the default seed if a default does not
     *  already exist.
     */
    auto Wallet_ImportSeed(
        const std::string& words,
        const std::string& passphrase) const -> std::string;

    OPENTXS_NO_EXPORT ~OTAPI_Exec() override = default;

private:
    friend api::client::implementation::Manager;

    const api::Core& api_;
    const api::client::Activity& activity_;
    const api::client::Contacts& contacts_;
    const api::network::ZMQ& zeromq_;
    const OT_API& ot_api_;
    ContextLockCallback lock_callback_;

    OTAPI_Exec(
        const api::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts,
        const api::network::ZMQ& zeromq,
        const OT_API& otapi,
        const ContextLockCallback& lockCallback);
    OTAPI_Exec() = delete;
    OTAPI_Exec(const OTAPI_Exec&) = delete;
    OTAPI_Exec(OTAPI_Exec&&) = delete;
    auto operator=(const OTAPI_Exec&) -> OTAPI_Exec = delete;
    auto operator=(OTAPI_Exec&&) -> OTAPI_Exec = delete;
};
}  // namespace opentxs
#endif

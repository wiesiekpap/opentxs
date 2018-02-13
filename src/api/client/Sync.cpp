/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/ServerAction.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"

#include <chrono>

#include "Sync.hpp"

#define CONTACT_REFRESH_DAYS 1
#define CONTRACT_DOWNLOAD_SECONDS 10
#define MAIN_LOOP_SECONDS 5
#define NYM_REGISTRATION_SECONDS 10

#define SHUTDOWN()                                                             \
    {                                                                          \
        if (shutdown_.load()) {                                                \
                                                                               \
            return;                                                            \
        }                                                                      \
                                                                               \
        Log::Sleep(std::chrono::milliseconds(50));                             \
    }

#define CHECK_NYM(a)                                                           \
    {                                                                          \
        if (a.empty()) {                                                       \
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid " << #a           \
                  << std::endl;                                                \
                                                                               \
            return {};                                                         \
        }                                                                      \
    }

#define CHECK_SERVER(a, b)                                                     \
    {                                                                          \
        CHECK_NYM(a)                                                           \
                                                                               \
        if (b.empty()) {                                                       \
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid " << #b           \
                  << std::endl;                                                \
                                                                               \
            return {};                                                         \
        }                                                                      \
    }

#define CHECK_ARGS(a, b, c)                                                    \
    {                                                                          \
        CHECK_SERVER(a, b)                                                     \
                                                                               \
        if (c.empty()) {                                                       \
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid " << #c           \
                  << std::endl;                                                \
                                                                               \
            return {};                                                         \
        }                                                                      \
    }

#define INTRODUCTION_SERVER_KEY "introduction_server_id"
#define MASTER_SECTION "Master"
#define PROCESS_INBOX_RETRIES 3

#define OT_METHOD "opentxs::api::client::implementation::Sync::"

namespace opentxs::api::client::implementation
{

const std::string Sync::DEFAULT_INTRODUCTION_SERVER =
    R"(-----BEGIN OT ARMORED SERVER CONTRACT-----
Version: Open Transactions 0.99.1-113-g2b3acf5
Comment: http://opentransactions.org

CAESI290b20xcHFmREJLTmJLR3RiN0NBa0ZodFRXVFVOTHFIRzIxGiNvdHVkd3p4cWF0UHh4
bmh4VFV3RUo3am5HenE2RkhGYTRraiIMU3Rhc2ggQ3J5cHRvKr8NCAESI290dWR3enhxYXRQ
eHhuaHhUVXdFSjdqbkd6cTZGSEZhNGtqGAIoATJTCAEQAiJNCAESIQI9MywLxxKfOtai26pj
JbxKtCCPhM/DbvX08iwbW2qYqhoga6Ccvp6CABGAFj/RdWNjtg5uzIRHT5Dn+fUzdAM9SUSA
AQCIAQA6vAwIARIjb3R1ZHd6eHFhdFB4eG5oeFRVd0VKN2puR3pxNkZIRmE0a2oaI290dXdo
ZzNwb2kxTXRRdVkzR3hwYWpOaXp5bmo0NjJ4Z2RIIAIymgQIARIjb3R1d2hnM3BvaTFNdFF1
WTNHeHBhak5penluajQ2MnhnZEgYAiABKAIyI290dWR3enhxYXRQeHhuaHhUVXdFSjdqbkd6
cTZGSEZhNGtqQl0IARJTCAEQAiJNCAESIQI9MywLxxKfOtai26pjJbxKtCCPhM/DbvX08iwb
W2qYqhoga6Ccvp6CABGAFj/RdWNjtg5uzIRHT5Dn+fUzdAM9SUSAAQCIAQAaBAgBEAJKiAEI
ARACGioIARAEGAIgASogZ6MtTp4aTEDLxFfhnsGo+Esp5B4hkgjWEejNPt5J6C0aKggBEAQY
AiACKiAhqJjWf2Ugqbg6z6ps59crAx9lHwTuT6Eq4x6JmkBlGBoqCAEQBBgCIAMqII2Vps1F
C2YUMbB4kE9XsHt1jrVY6pMPV6KWc5sH3VvTem0IARIjb3R1d2hnM3BvaTFNdFF1WTNHeHBh
ak5penluajQ2MnhnZEgYASAFKkDQLsszAol/Ih56MomuBKV8zpKaw5+ry7Kse1+5nPwJlP8f
72OAgTegBlmv31K4JgLVs52EKJTBpjnV+v0pxzUOem0IARIjb3R1ZHd6eHFhdFB4eG5oeFRV
d0VKN2puR3pxNkZIRmE0a2oYAyAFKkAJZ0LTVM+XBrGbRdiZsEQSbvwqg+mqGwHD5MQ+D4h0
fPQaUrdB6Pp/HM5veox02LBKg05hVNQ64tcU+LAxK+VHQuQDCAESI290clA2dDJXY2hYMjYz
ZVpiclRuVzZyY2FCZVNQb2VqSzJnGAIgAigCMiNvdHVkd3p4cWF0UHh4bmh4VFV3RUo3am5H
enE2RkhGYTRrajonCAESI290dXdoZzNwb2kxTXRRdVkzR3hwYWpOaXp5bmo0NjJ4Z2RISogB
CAEQAhoqCAEQBBgCIAEqIDpwlCrxHNWvvtFt6k8ocB5NBo7vjkGO/mRuSOQ/j/9WGioIARAE
GAIgAiog6Dw0+AWok4dENWWc/3qhykA7NNybWecqMGs5fL8KLLYaKggBEAQYAiADKiD+s/iq
37NrYI4/xdHOYtO/ocR0YqDVz09IaDNGVEdBtnptCAESI290clA2dDJXY2hYMjYzZVpiclRu
VzZyY2FCZVNQb2VqSzJnGAEgBSpATbHtakma53Na35Be+rGvW+z1H6EtkHlljv9Mo8wfies3
in9el1Ejb4BDbGCN5ABl3lQpfedZnR+VYv2X6Y1yBnptCAESI290dXdoZzNwb2kxTXRRdVkz
R3hwYWpOaXp5bmo0NjJ4Z2RIGAEgBSpAeptEmgdqgkGUcOJCqG0MsiChEREUdDzH/hRj877u
WDIHoRHsf/k5dCOHfDct4TDszasVhGFhRdNunpgQJcp0DULnAwgBEiNvdHd6ZWd1dTY3cENI
RnZhYjZyS2JYaEpXelNvdlNDTGl5URgCIAIoAjIjb3R1ZHd6eHFhdFB4eG5oeFRVd0VKN2pu
R3pxNkZIRmE0a2o6JwgBEiNvdHV3aGczcG9pMU10UXVZM0d4cGFqTml6eW5qNDYyeGdkSEqL
AQgBEAIaKwgBEAMYAiABKiEC5p36Ivxs4Wb6CjKTnDA1MFtX3Mx2UBlrmloSt+ffXz0aKwgB
EAMYAiACKiECtMkEo4xsefeevzrBb62ll98VYZy8PipgbrPWqGUNxQMaKwgBEAMYAiADKiED
W1j2DzOZemB9OOZ/pPrFroKDfgILYu2IOtiRFfi0vDB6bQgBEiNvdHd6ZWd1dTY3cENIRnZh
YjZyS2JYaEpXelNvdlNDTGl5URgBIAUqQJYd860/Ybh13GtW+grxWtWjjmzPifHE7bTlgUWl
3bX+ZuWNeEotA4yXQvFNog4PTAOF6dbvCr++BPGepBEUEEx6bQgBEiNvdHV3aGczcG9pMU10
UXVZM0d4cGFqTml6eW5qNDYyeGdkSBgBIAUqQH6GXnKCCDDgDvcSt8dLWuVMlr75zVkHy85t
tccoy2oLHNevDvKrLfUk/zuICyaSIvDy0Kb2ytOuh/O17yabxQ8yHQgBEAEYASISb3Quc3Rh
c2hjcnlwdG8ubmV0KK03MiEIARADGAEiFnQ1NGxreTJxM2w1ZGt3bnQub25pb24orTcyRwgB
EAQYASI8b3ZpcDZrNWVycXMzYm52cjU2cmgzZm5pZ2JuZjJrZWd1cm5tNWZpYnE1NWtqenNv
YW54YS5iMzIuaTJwKK03Op8BTWVzc2FnaW5nLW9ubHkgc2VydmVyIHByb3ZpZGVkIGZvciB0
aGUgY29udmllbmllbmNlIG9mIFN0YXNoIENyeXB0byB1c2Vycy4gU2VydmljZSBpcyBwcm92
aWRlZCBhcyBpcyB3aXRob3V0IHdhcnJhbnR5IG9mIGFueSBraW5kLCBlaXRoZXIgZXhwcmVz
c2VkIG9yIGltcGxpZWQuQiCK4L5cnecfUFz/DQyvAklKC2pTmWQtxt9olQS5/0hUHUptCAES
I290clA2dDJXY2hYMjYzZVpiclRuVzZyY2FCZVNQb2VqSzJnGAUgBSpA1/bep0NTbisZqYns
MCL/PCUJ6FIMhej+ROPk41604x1jeswkkRmXRNjzLlVdiJ/pQMxG4tJ0UQwpxHxrr0IaBA==
-----END OT ARMORED SERVER CONTRACT-----)";

Sync::Sync(
    std::recursive_mutex& apiLock,
    const std::atomic<bool>& shutdown,
    const OT_API& otapi,
    const opentxs::OTAPI_Exec& exec,
    const api::ContactManager& contacts,
    const api::Settings& config,
    const api::client::ServerAction& serverAction,
    const api::client::Wallet& wallet,
    const api::crypto::Encode& encoding)
    : api_lock_(apiLock)
    , shutdown_(shutdown)
    , ot_api_(otapi)
    , exec_(exec)
    , contacts_(contacts)
    , config_(config)
    , server_action_(serverAction)
    , wallet_(wallet)
    , encoding_(encoding)
    , introduction_server_lock_()
    , task_status_lock_()
    , refresh_counter_(0)
    , operations_()
    , server_nym_fetch_()
    , missing_nyms_()
    , missing_servers_()
    , state_machines_()
    , introduction_server_id_()
    , task_status_()
{
}

std::pair<bool, std::size_t> Sync::accept_incoming(
    const rLock& lock[[maybe_unused]],
    const std::size_t max,
    const Identifier& accountID,
    ServerContext& context) const
{
    std::pair<bool, std::size_t> output{false, 0};
    auto & [ success, remaining ] = output;
    const std::string account = String(accountID).Get();
    auto processInbox = ot_api_.CreateProcessInbox(accountID, context);
    auto& response = std::get<0>(processInbox);
    auto& inbox = std::get<1>(processInbox);

    if (false == bool(response)) {
        if (nullptr == inbox) {
            // This is a new account which has never instantiated an inbox.
            success = true;

            return output;
        }

        otErr << OT_METHOD << __FUNCTION__
              << ": Error instantiating processInbox for account: " << account
              << std::endl;

        return output;
    }

    const std::size_t items =
        (inbox->GetTransactionCount() >= 0) ? inbox->GetTransactionCount() : 0;
    const std::size_t count = (items > max) ? max : items;
    remaining = items - count;

    if (0 == count) {
        otInfo << OT_METHOD << __FUNCTION__
               << ": No items to accept in this account." << std::endl;
        success = true;

        return output;
    }

    for (std::size_t i = 0; i < count; i++) {
        auto transaction = inbox->GetTransactionByIndex(i);

        OT_ASSERT(nullptr != transaction);

        const TransactionNumber number = transaction->GetTransactionNum();

        if (transaction->IsAbbreviated()) {
            inbox->LoadBoxReceipt(number);
            transaction = inbox->GetTransaction(number);

            if (nullptr == transaction) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unable to load item: " << number << std::endl;

                continue;
            }
        }

        const bool accepted = ot_api_.IncludeResponse(
            accountID, true, context, *transaction, *response);

        if (!accepted) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to accept item: " << number << std::endl;

            return output;
        }
    }

    const bool finalized =
        ot_api_.FinalizeProcessInbox(accountID, context, *response, *inbox);

    if (false == finalized) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to finalize response."
              << std::endl;

        return output;
    }

    auto action = server_action_.ProcessInbox(
        context.Nym()->ID(), context.Server(), accountID, *response);
    action->Run();
    success = (SendResult::VALID_REPLY == action->LastSendResult());

    return output;
}

bool Sync::AcceptIncoming(
    const Identifier& nymID,
    const Identifier& accountID,
    const Identifier& serverID,
    const std::size_t max) const
{
    rLock apiLock(api_lock_);
    auto context = wallet_.mutable_ServerContext(nymID, serverID);
    std::size_t remaining{1};
    std::size_t retries{PROCESS_INBOX_RETRIES};

    while (0 < remaining) {
        const auto attempt =
            accept_incoming(apiLock, max, accountID, context.It());
        const auto & [ success, unprocessed ] = attempt;
        remaining = unprocessed;

        if (false == success) {
            if (0 == retries) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Exceeded maximum retries." << std::endl;

                return false;
            }

            Utility utility(context.It(), ot_api_);
            const auto download = utility.getIntermediaryFiles(
                String(context.It().Server()).Get(),
                String(context.It().Nym()->ID()).Get(),
                String(accountID).Get(),
                true);

            if (false == download) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed to download account files." << std::endl;

                return false;
            } else {
                --retries;

                continue;
            }
        }

        if (0 != remaining) {
            otErr << OT_METHOD << __FUNCTION__ << ": Accepting " << remaining
                  << " more items." << std::endl;
        }
    }

    return true;
}

void Sync::add_task(const Identifier& taskID, const ThreadStatus status) const
{
    Lock lock(task_status_lock_);

    if (0 != task_status_.count(taskID)) {

        return;
    }

    task_status_[taskID] = status;
}

Messagability Sync::can_message(
    const Identifier& senderNymID,
    const Identifier& recipientContactID,
    Identifier& recipientNymID,
    Identifier& serverID) const
{
    auto senderNym = wallet_.Nym(senderNymID);

    if (false == bool(senderNym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load sender nym "
              << String(senderNymID) << std::endl;

        return Messagability::MISSING_SENDER;
    }

    const bool canSign = senderNym->hasCapability(NymCapability::SIGN_MESSAGE);

    if (false == canSign) {
        otErr << OT_METHOD << __FUNCTION__ << ": Sender nym "
              << String(senderNymID)
              << " can not sign messages (no private key)." << std::endl;

        return Messagability::INVALID_SENDER;
    }

    const auto contact = contacts_.Contact(recipientContactID);

    if (false == bool(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << String(recipientContactID) << " does not exist." << std::endl;

        return Messagability::MISSING_CONTACT;
    }

    const auto nyms = contact->Nyms();

    if (0 == nyms.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << String(recipientContactID) << " does not have a nym."
              << std::endl;

        return Messagability::CONTACT_LACKS_NYM;
    }

    std::shared_ptr<const Nym> recipientNym{nullptr};

    for (const auto& it : nyms) {
        recipientNym = wallet_.Nym(it);

        if (recipientNym) {
            recipientNymID = it;
            break;
        }
    }

    if (false == bool(recipientNym)) {
        for (const auto& id : nyms) {
            missing_nyms_.Push({}, id);
        }

        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << String(recipientContactID) << " credentials not available."
              << std::endl;

        return Messagability::MISSING_RECIPIENT;
    }

    const auto claims = recipientNym->Claims();
    serverID = claims.PreferredOTServer();

    // TODO maybe some of the other nyms in this contact do specify a server
    if (serverID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << String(recipientContactID) << ", nym "
              << String(recipientNymID)
              << ": credentials do not specify a server." << std::endl;
        missing_nyms_.Push({}, recipientNymID);

        return Messagability::NO_SERVER_CLAIM;
    }

    const bool registered = exec_.IsNym_RegisteredAtServer(
        String(senderNymID).Get(), String(serverID).Get());

    if (false == registered) {
        ScheduleDownloadNymbox(senderNymID, serverID);
        otErr << OT_METHOD << __FUNCTION__ << ": Sender nym "
              << String(senderNymID) << " not registered on server "
              << String(serverID) << std::endl;

        return Messagability::UNREGISTERED;
    }

    return Messagability::READY;
}

Messagability Sync::CanMessage(
    const Identifier& senderNymID,
    const Identifier& recipientContactID) const
{
    if (senderNymID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid sender" << std::endl;

        return Messagability::INVALID_SENDER;
    }

    if (recipientContactID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid recipient"
              << std::endl;

        return Messagability::MISSING_CONTACT;
    }

    Identifier nymID{};
    Identifier serverID{};
    Lock lock(introduction_server_lock_);
    start_introduction_server(lock, senderNymID);
    lock.unlock();

    return can_message(senderNymID, recipientContactID, nymID, serverID);
}

void Sync::check_nym_revision(
    const ServerContext& context,
    OperationQueue& queue) const
{
    if (context.StaleNym()) {
        const auto& nymID = context.Nym()->ID();
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << String(nymID)
              << " has is newer than version last registered version on server "
              << String(context.Server()) << std::endl;
        queue.register_nym_.Push({}, true);
    }
}

bool Sync::check_registration(
    const Identifier& nymID,
    const Identifier& serverID,
    std::shared_ptr<const ServerContext>& context) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    context = wallet_.ServerContext(nymID, serverID);
    RequestNumber request{0};

    if (context) {
        request = context->Request();
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << String(nymID)
              << " has never registered on " << String(serverID) << std::endl;
    }

    if (0 != request) {
        OT_ASSERT(context)

        return true;
    }

    const auto output = register_nym({}, nymID, serverID);

    if (output) {
        context = wallet_.ServerContext(nymID, serverID);

        OT_ASSERT(context)
    }

    return output;
}

bool Sync::check_server_contract(const Identifier& serverID) const
{
    OT_ASSERT(false == serverID.empty())

    const auto serverContract = wallet_.Server(serverID);

    if (serverContract) {

        return true;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Server contract for "
          << String(serverID) << " is not in the wallet." << std::endl;
    missing_servers_.Push({}, serverID);

    return false;
}

bool Sync::download_account(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& accountID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == accountID.empty())

    const auto success =
        server_action_.DownloadAccount(nymID, serverID, accountID);

    return finish_task(taskID, success);
}

bool Sync::download_contract(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& contractID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == contractID.empty())

    rLock lock(api_lock_);
    auto action = server_action_.DownloadContract(nymID, serverID, contractID);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server "
                  << String(serverID) << " does not have the contract "
                  << String(contractID) << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while downloading contract "
              << String(contractID) << " from server " << String(serverID)
              << std::endl;
    }

    return finish_task(taskID, false);
}

bool Sync::download_nym(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetNymID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == targetNymID.empty())

    rLock lock(api_lock_);
    auto action = server_action_.DownloadNym(nymID, serverID, targetNymID);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server "
                  << String(serverID) << " does not have nym "
                  << String(targetNymID) << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while downloading nym "
              << String(targetNymID) << " from server " << String(serverID)
              << std::endl;
    }

    return finish_task(taskID, false);
}

bool Sync::download_nymbox(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    const auto success = server_action_.DownloadNymbox(nymID, serverID);

    return finish_task(taskID, success);
}

bool Sync::find_nym(
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == targetID.empty())

    const auto nym = wallet_.Nym(targetID);

    if (nym) {
        missing_nyms_.CancelByValue(targetID);

        return true;
    }

    if (download_nym({}, nymID, serverID, targetID)) {
        missing_nyms_.CancelByValue(targetID);

        return true;
    }

    return false;
}

bool Sync::find_server(
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == targetID.empty())

    const auto serverContract = wallet_.Server(targetID);

    if (serverContract) {
        missing_servers_.CancelByValue(targetID);

        return true;
    }

    if (download_contract({}, nymID, serverID, targetID)) {
        missing_servers_.CancelByValue(targetID);

        return true;
    }

    return false;
}

Identifier Sync::FindNym(const Identifier& nymID) const
{
    CHECK_NYM(nymID)

    const auto taskID(random_id());

    return start_task(taskID, missing_nyms_.Push(taskID, nymID));
}

Identifier Sync::FindNym(
    const Identifier& nymID,
    const Identifier& serverIDHint) const
{
    CHECK_NYM(nymID)

    Lock lock(lock_);
    auto& serverQueue = server_nym_fetch_[serverIDHint];
    lock.unlock();
    const auto taskID(random_id());

    return start_task(taskID, serverQueue.Push(taskID, nymID));
}

Identifier Sync::FindServer(const Identifier& serverID) const
{
    CHECK_NYM(serverID)

    const auto taskID(random_id());

    return start_task(taskID, missing_servers_.Push(taskID, serverID));
}

bool Sync::finish_task(const Identifier& taskID, const bool success) const
{
    if (success) {
        update_task(taskID, ThreadStatus::FINISHED_SUCCESS);
    } else {
        update_task(taskID, ThreadStatus::FINISHED_FAILED);
    }

    return success;
}

bool Sync::get_admin(
    const Identifier& nymID,
    const Identifier& serverID,
    const OTPassword& password) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    rLock lock(api_lock_);
    bool success{false};

    {
        const std::string serverPassword(password.getPassword());
        auto action =
            server_action_.RequestAdmin(nymID, serverID, serverPassword);
        action->Run();

        if (SendResult::VALID_REPLY == action->LastSendResult()) {
            auto reply = action->Reply();

            OT_ASSERT(reply)

            success = reply->m_bSuccess;
        }
    }

    auto mContext = wallet_.mutable_ServerContext(nymID, serverID);
    auto& context = mContext.It();
    context.SetAdminAttempted();

    if (success) {
        otErr << OT_METHOD << __FUNCTION__ << ": Got admin on server "
              << String(serverID) << std::endl;
        context.SetAdminSuccess();
    }

    return success;
}

Identifier Sync::get_introduction_server(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock, introduction_server_lock_))

    bool keyFound = false;
    String serverID;
    rLock apiLock(api_lock_);
    const bool config = config_.Check_str(
        MASTER_SECTION, INTRODUCTION_SERVER_KEY, serverID, keyFound);

    if (!config || !keyFound || !serverID.Exists()) {

        return import_default_introduction_server(lock);
    }

    return Identifier(String(serverID.Get()));
}

Sync::OperationQueue& Sync::get_operations(
    const Lock& lock,
    const ContextID& id) const
{
    OT_ASSERT(verify_lock(lock))

    auto& queue = operations_[id];
    auto& thread = state_machines_[id];

    if (false == bool(thread)) {
        thread.reset(new std::thread(
            [id, &queue, this]() { state_machine(id, queue); }));
    }

    return queue;
}

Identifier Sync::import_default_introduction_server(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock, introduction_server_lock_))

    const auto serialized = proto::StringToProto<proto::ServerContract>(
        DEFAULT_INTRODUCTION_SERVER.c_str());
    const auto instantiated = wallet_.Server(serialized);

    OT_ASSERT(instantiated)

    return set_introduction_server(lock, *instantiated);
}

const Identifier& Sync::IntroductionServer() const
{
    Lock lock(introduction_server_lock_);

    if (false == bool(introduction_server_id_)) {
        load_introduction_server();
    }

    OT_ASSERT(introduction_server_id_)

    return *introduction_server_id_;
}

void Sync::load_introduction_server() const
{
    Lock lock(introduction_server_lock_);
    introduction_server_id_.reset(
        new Identifier(get_introduction_server(lock)));
}

bool Sync::message_nym(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const std::string& text) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == targetNymID.empty())

    rLock lock(api_lock_);
    auto action =
        server_action_.SendMessage(nymID, serverID, targetNymID, text);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server  "
                  << String(serverID) << " does not accept message for "
                  << String(targetNymID) << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while messaging nym "
              << String(targetNymID) << " on server " << String(serverID)
              << std::endl;
    }

    return finish_task(taskID, false);
}

Identifier Sync::MessageContact(
    const Identifier& senderNymID,
    const Identifier& contactID,
    const std::string& message) const
{
    CHECK_SERVER(senderNymID, contactID)

    Lock introLock(introduction_server_lock_);
    start_introduction_server(introLock, senderNymID);
    introLock.unlock();
    Identifier serverID;
    Identifier recipientNymID;
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) {

        return {};
    }

    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == recipientNymID.empty())

    Lock lock(lock_);
    auto& queue = get_operations(lock, {senderNymID, serverID});
    const auto taskID(random_id());

    return start_task(
        taskID, queue.send_message_.Push(taskID, {recipientNymID, message}));
}

bool Sync::publish_server_registration(
    const Identifier& nymID,
    const Identifier& serverID,
    const bool forcePrimary) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    auto nym = wallet_.mutable_Nym(nymID);

    return nym.AddPreferredOTServer(String(serverID).Get(), forcePrimary);
}

Identifier Sync::random_id() const
{
    Identifier output;
    auto nonce = Data::Factory();
    encoding_.Nonce(32, nonce);
    output.CalculateDigest(nonce);

    return output;
}

void Sync::Refresh() const
{
    Lock lock(lock_);
    refresh_accounts(lock);
    lock.unlock();

    SHUTDOWN()

    refresh_contacts();
    ++refresh_counter_;
}

std::uint64_t Sync::RefreshCount() const { return refresh_counter_.load(); }

void Sync::refresh_accounts(const Lock& lock) const
{
    // Make sure no nyms, servers, or accounts are added or removed while
    // creating the list
    rLock apiLock(api_lock_);
    const auto serverList = wallet_.ServerList();
    const auto nymCount = exec_.GetNymCount();
    const auto accountCount = exec_.GetAccountCount();

    for (const auto server : serverList) {
        SHUTDOWN()

        const auto& serverID = server.first;

        for (std::int32_t n = 0; n < nymCount; n++) {
            SHUTDOWN()

            const auto nymID = exec_.GetNym_ID(n);

            if (exec_.IsNym_RegisteredAtServer(nymID, serverID)) {
                auto& queue = get_operations(
                    lock, {Identifier(nymID), Identifier(serverID)});
                const auto taskID(random_id());
                queue.download_nymbox_.Push(taskID, true);
            }
        }
    }

    SHUTDOWN()

    for (std::int32_t n = 0; n < accountCount; n++) {
        SHUTDOWN()

        const auto accountID = exec_.GetAccountWallet_ID(n);
        const auto serverID = exec_.GetAccountWallet_NotaryID(accountID);
        const auto nymID = exec_.GetAccountWallet_NymID(accountID);
        auto& queue =
            get_operations(lock, {Identifier(nymID), Identifier(serverID)});
        const auto taskID(random_id());
        queue.download_account_.Push(taskID, Identifier(accountID));
    }
}

void Sync::refresh_contacts() const
{
    for (const auto& it : contacts_.ContactList()) {
        SHUTDOWN()

        const auto& contactID = it.first;
        otInfo << OT_METHOD << __FUNCTION__
               << ": Considering contact: " << contactID << std::endl;
        const auto contact = contacts_.Contact(Identifier(contactID));

        OT_ASSERT(contact);

        const auto now = std::time(nullptr);
        const std::chrono::seconds interval(now - contact->LastUpdated());
        const std::chrono::hours limit(24 * CONTACT_REFRESH_DAYS);
        const auto nymList = contact->Nyms();

        if (nymList.empty()) {
            otInfo << OT_METHOD << __FUNCTION__
                   << ": No nyms associated with this contact." << std::endl;

            continue;
        }

        for (const auto& nymID : nymList) {
            SHUTDOWN()

            const auto nym = wallet_.Nym(nymID);
            otInfo << OT_METHOD << __FUNCTION__
                   << ": Considering nym: " << String(nymID) << std::endl;

            if (nym) {
                contacts_.Update(nym->asPublicNym());
            } else {
                otInfo << OT_METHOD << __FUNCTION__
                       << ": We don't have credentials for this nym. "
                       << " Will search on all servers." << std::endl;
                const auto taskID(random_id());
                missing_nyms_.Push(taskID, nymID);

                continue;
            }

            if (interval > limit) {
                otInfo << OT_METHOD << __FUNCTION__
                       << ": Hours since last update (" << interval.count()
                       << ") exceeds the limit (" << limit.count() << ")"
                       << std::endl;
                // TODO add a method to Contact that returns the list of
                // servers
                const auto data = contact->Data();

                if (false == bool(data)) {

                    continue;
                }

                const auto serverGroup = data->Group(
                    proto::CONTACTSECTION_COMMUNICATION,
                    proto::CITEMTYPE_OPENTXS);

                if (false == bool(serverGroup)) {

                    continue;
                }

                for (const auto & [ claimID, item ] : *serverGroup) {
                    SHUTDOWN()
                    OT_ASSERT(item)

                    const auto& notUsed[[maybe_unused]] = claimID;
                    const Identifier serverID(item->Value());

                    if (serverID.empty()) {

                        continue;
                    }

                    otInfo << OT_METHOD << __FUNCTION__
                           << ": Will download nym " << String(nymID)
                           << " from server " << String(serverID) << std::endl;
                    Lock lock(lock_);
                    auto& serverQueue = server_nym_fetch_[serverID];
                    lock.unlock();
                    const auto taskID(random_id());
                    serverQueue.Push(taskID, nymID);
                }
            } else {
                otInfo << OT_METHOD << __FUNCTION__
                       << ": No need to update this nym." << std::endl;
            }
        }
    }
}

bool Sync::register_nym(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    rLock lock(api_lock_);
    auto action = server_action_.RegisterNym(nymID, serverID);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server "
                  << String(serverID) << " did not accept registration for nym "
                  << String(nymID) << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while registering nym " << String(nymID)
              << " on server " << String(serverID) << std::endl;
    }

    return finish_task(taskID, false);
}

Identifier Sync::RegisterNym(
    const Identifier& nymID,
    const Identifier& serverID,
    const bool setContactData) const
{
    CHECK_SERVER(nymID, serverID)

    Lock introLock(introduction_server_lock_);
    start_introduction_server(introLock, nymID);
    introLock.unlock();

    if (setContactData) {
        publish_server_registration(nymID, serverID, false);
    }

    return ScheduleRegisterNym(nymID, serverID);
}

Identifier Sync::SetIntroductionServer(const ServerContract& contract) const
{
    Lock lock(introduction_server_lock_);

    return set_introduction_server(lock, contract);
}

Identifier Sync::ScheduleDownloadAccount(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID) const
{
    CHECK_ARGS(localNymID, serverID, accountID)

    Lock introLock(introduction_server_lock_);
    start_introduction_server(introLock, localNymID);
    introLock.unlock();
    Lock lock(lock_);
    auto& queue = get_operations(lock, {localNymID, serverID});
    const auto taskID(random_id());

    return start_task(taskID, queue.download_account_.Push(taskID, accountID));
}

Identifier Sync::ScheduleDownloadContract(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& contractID) const
{
    CHECK_ARGS(localNymID, serverID, contractID)

    Lock introLock(introduction_server_lock_);
    start_introduction_server(introLock, localNymID);
    introLock.unlock();
    Lock lock(lock_);
    auto& queue = get_operations(lock, {localNymID, serverID});
    const auto taskID(random_id());

    return start_task(
        taskID, queue.download_contract_.Push(taskID, contractID));
}

Identifier Sync::ScheduleDownloadNym(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID) const
{
    CHECK_ARGS(localNymID, serverID, targetNymID)

    Lock introLock(introduction_server_lock_);
    start_introduction_server(introLock, localNymID);
    introLock.unlock();
    Lock lock(lock_);
    auto& queue = get_operations(lock, {localNymID, serverID});
    const auto taskID(random_id());

    return start_task(taskID, queue.check_nym_.Push(taskID, targetNymID));
}

Identifier Sync::ScheduleDownloadNymbox(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    CHECK_SERVER(localNymID, serverID)

    Lock introLock(introduction_server_lock_);
    start_introduction_server(introLock, localNymID);
    introLock.unlock();
    Lock lock(lock_);
    auto& queue = get_operations(lock, {localNymID, serverID});
    const auto taskID(random_id());

    return start_task(taskID, queue.download_nymbox_.Push(taskID, true));
}

Identifier Sync::ScheduleRegisterNym(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    CHECK_SERVER(localNymID, serverID)

    Lock introLock(introduction_server_lock_);
    start_introduction_server(introLock, localNymID);
    introLock.unlock();
    Lock lock(lock_);
    auto& queue = get_operations(lock, {localNymID, serverID});
    const auto taskID(random_id());

    return start_task(taskID, queue.register_nym_.Push(taskID, true));
}

Identifier Sync::set_introduction_server(
    const Lock& lock,
    const ServerContract& contract) const
{
    OT_ASSERT(verify_lock(lock, introduction_server_lock_));

    auto instantiated = wallet_.Server(contract.PublicContract());

    if (false == bool(instantiated)) {

        return {};
    }

    const auto& id = instantiated->ID();
    introduction_server_id_.reset(new Identifier(id));

    OT_ASSERT(introduction_server_id_)
    bool dontCare = false;
    rLock apiLock(api_lock_);
    const bool set = config_.Set_str(
        MASTER_SECTION, INTRODUCTION_SERVER_KEY, String(id), dontCare);

    OT_ASSERT(set)
    config_.Save();

    return id;
}

void Sync::start_introduction_server(const Lock& lock, const Identifier& nymID)
    const
{
    auto serverID = get_introduction_server(lock);

    if (serverID.empty()) {

        return;
    }

    Lock objectLock(lock_);
    auto& queue = get_operations(objectLock, {nymID, serverID});
    objectLock.unlock();
    const auto taskID(random_id());
    start_task(taskID, queue.download_nymbox_.Push(taskID, true));
}

Identifier Sync::start_task(const Identifier& taskID, bool success) const
{
    if (taskID.empty()) {

        return {};
    }

    if (false == success) {

        return {};
    }

    add_task(taskID, ThreadStatus::RUNNING);

    return taskID;
}

void Sync::state_machine(const ContextID id, OperationQueue& queue) const
{
    const auto & [ nymID, serverID ] = id;

    // Make sure the server contract is available
    while (false == shutdown_.load()) {
        if (check_server_contract(serverID)) {
            otInfo << OT_METHOD << __FUNCTION__ << ": Server contract "
                   << String(serverID) << " exists." << std::endl;

            break;
        }

        Log::Sleep(std::chrono::seconds(CONTRACT_DOWNLOAD_SECONDS));
    }

    SHUTDOWN()

    std::shared_ptr<const ServerContext> context{nullptr};

    // Make sure the nym has registered for the first time on the server
    while (false == shutdown_.load()) {
        if (check_registration(nymID, serverID, context)) {
            otInfo << OT_METHOD << __FUNCTION__ << ": Nym " << String(nymID)
                   << " has registered on server " << String(serverID)
                   << " at least once." << std::endl;

            break;
        }

        Log::Sleep(std::chrono::seconds(NYM_REGISTRATION_SECONDS));
    }

    SHUTDOWN()
    OT_ASSERT(context)

    bool queueValue{false};
    bool needAdmin{false};
    bool registerNym{false};
    bool registerNymQueued{false};
    bool downloadNymbox{false};
    Identifier taskID{};
    Identifier accountID{};
    Identifier contractID{};
    Identifier targetNymID{};
    OTPassword serverPassword;
    MessageTask message;

    // Primary loop
    while (false == shutdown_.load()) {
        // If the local nym has updated since the last registernym operation,
        // schedule a registernym
        check_nym_revision(*context, queue);

        SHUTDOWN()

        // Register the nym, if scheduled. Keep trying until success
        registerNym |= queueValue;
        registerNymQueued = queue.register_nym_.Pop(taskID, queueValue);

        if (registerNymQueued || registerNym) {
            registerNym |= !register_nym(taskID, nymID, serverID);
        }

        SHUTDOWN()

        // If this server was added by a pairing operation that included
        // a server password then request admin permissions on the server
        needAdmin =
            context->HaveAdminPassword() && (false == context->isAdmin());

        if (needAdmin) {
            serverPassword.setPassword(context->AdminPassword());
            get_admin(nymID, serverID, serverPassword);
        }

        SHUTDOWN()

        // This is a list of servers for which we do not have a contract.
        // We ask all known servers on which we are registered to try to find
        // the contracts.
        const auto servers = missing_servers_.Copy();

        for (const auto & [ targetID, taskID ] : servers) {
            SHUTDOWN()

            if (targetID.empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty serverID get in here?"
                      << std::endl;

                continue;
            } else {
                otWarn << OT_METHOD << __FUNCTION__
                       << ": Searching for server contract for "
                       << String(targetID) << std::endl;
            }

            const auto& notUsed[[maybe_unused]] = taskID;
            find_server(nymID, serverID, targetID);
        }

        // This is a list of contracts (server and unit definition) which a
        // user of this class has requested we download from this server.
        while (queue.download_contract_.Pop(taskID, contractID)) {
            SHUTDOWN()

            if (contractID.empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty contract ID get in here?"
                      << std::endl;

                continue;
            } else {
                otWarn << OT_METHOD << __FUNCTION__
                       << ": Searching for unit definition contract for "
                       << String(contractID) << std::endl;
            }

            download_contract(taskID, nymID, serverID, contractID);
        }

        // This is a list of nyms for which we do not have credentials..
        // We ask all known servers on which we are registered to try to find
        // their credentials.
        const auto nyms = missing_nyms_.Copy();

        for (const auto & [ targetID, taskID ] : nyms) {
            SHUTDOWN()

            if (targetID.empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty nymID get in here?" << std::endl;

                continue;
            } else {
                otWarn << OT_METHOD << __FUNCTION__ << ": Searching for nym "
                       << String(targetID) << std::endl;
            }

            const auto& notUsed[[maybe_unused]] = taskID;
            find_nym(nymID, serverID, targetID);
        }

        // This is a list of nyms which haven't been updated in a while and
        // are known or suspected to be available on this server
        Lock lock(lock_);
        auto& nymQueue = server_nym_fetch_[serverID];
        lock.unlock();

        while (nymQueue.Pop(taskID, targetNymID)) {
            SHUTDOWN()

            if (targetNymID.empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty nymID get in here?" << std::endl;

                continue;
            } else {
                otWarn << OT_METHOD << __FUNCTION__ << ": Refreshing nym "
                       << String(targetNymID) << std::endl;
            }

            download_nym(taskID, nymID, serverID, targetNymID);
        }

        // This is a list of nyms which a user of this class has requested we
        // download from this server.
        while (queue.check_nym_.Pop(taskID, targetNymID)) {
            SHUTDOWN()

            if (targetNymID.empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty nymID get in here?" << std::endl;

                continue;
            } else {
                otWarn << OT_METHOD << __FUNCTION__ << ": Searching for nym "
                       << String(targetNymID) << std::endl;
            }

            download_nym(taskID, nymID, serverID, targetNymID);
        }

        // This is a list of messages which need to be delivered to a nym
        // on this server
        while (queue.send_message_.Pop(taskID, message)) {
            SHUTDOWN()

            const auto & [ recipientID, text ] = message;

            if (recipientID.empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty recipient nymID get in here?"
                      << std::endl;

                continue;
            }

            message_nym(taskID, nymID, serverID, recipientID, text);
        }

        // Download the nymbox, if this operation has been scheduled
        if (queue.download_nymbox_.Pop(taskID, downloadNymbox)) {
            otWarn << OT_METHOD << __FUNCTION__ << ": Downloading nymbox for "
                   << String(nymID) << " on " << String(serverID) << std::endl;
            registerNym |= !download_nymbox(taskID, nymID, serverID);
        }

        SHUTDOWN()

        // Download any accounts which have been scheduled for download
        while (queue.check_nym_.Pop(taskID, accountID)) {
            SHUTDOWN()

            if (accountID.empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty account ID get in here?"
                      << std::endl;

                continue;
            } else {
                otWarn << OT_METHOD << __FUNCTION__ << ": Downloading account "
                       << String(accountID) << " for " << String(nymID)
                       << " on " << String(serverID) << std::endl;
            }

            registerNym |=
                !download_account(taskID, nymID, serverID, accountID);
        }

        Log::Sleep(std::chrono::seconds(MAIN_LOOP_SECONDS));
    }
}

ThreadStatus Sync::Status(const Identifier& taskID) const
{
    if (shutdown_.load()) {

        return ThreadStatus::SHUTDOWN;
    }

    Lock lock(task_status_lock_);
    auto it = task_status_.find(taskID);

    if (task_status_.end() == it) {

        return ThreadStatus::ERROR;
    }

    const auto output = it->second;
    const bool success = (ThreadStatus::FINISHED_SUCCESS == output);
    const bool failed = (ThreadStatus::FINISHED_FAILED == output);
    const bool finished = (success || failed);

    if (finished) {
        task_status_.erase(it);
    }

    return output;
}

void Sync::update_task(const Identifier& taskID, const ThreadStatus status)
    const
{
    if (taskID.empty()) {

        return;
    }

    Lock lock(task_status_lock_);

    if (0 == task_status_.count(taskID)) {

        return;
    }

    task_status_[taskID] = status;
}

Sync::~Sync()
{
    for (auto & [ id, thread ] : state_machines_) {
        const auto& notUsed[[maybe_unused]] = id;

        OT_ASSERT(thread)

        if (thread->joinable()) {
            thread->join();
        }
    }
}
}  // namespace opentxs::api::implementation

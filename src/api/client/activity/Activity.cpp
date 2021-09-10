// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "opentxs/api/client/Activity.hpp"  // IWYU pragma: associated

#include "api/client/activity/Imp.hpp"
#include "internal/api/client/Factory.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/core/Cheque.hpp"  // IWYU pragma: keep
#include "opentxs/core/Item.hpp"    // IWYU pragma: keep
#include "opentxs/core/Message.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"

// #define OT_METHOD "opentxs::api::client::implementation::Activity::"

namespace opentxs::factory
{
auto ActivityAPI(
    const api::Core& api,
    const api::client::Contacts& contact) noexcept
    -> std::unique_ptr<api::client::Activity>
{
    using ReturnType = api::client::Activity;

    return std::make_unique<ReturnType>(api, contact);
}
}  // namespace opentxs::factory

namespace opentxs::api::client
{
Activity::Activity(
    const api::Core& api,
    const client::Contacts& contact) noexcept
    : imp_(std::make_unique<Imp>(api, contact))
{
}

auto Activity::AddBlockchainTransaction(
    const Blockchain& api,
    const BlockchainTransaction& transaction) const noexcept -> bool
{
    return imp_->AddBlockchainTransaction(api, transaction);
}

auto Activity::AddPaymentEvent(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    const StorageBox type,
    const Identifier& itemID,
    const Identifier& workflowID,
    Time time) const noexcept -> bool
{
    return imp_->AddPaymentEvent(
        nymID, threadID, type, itemID, workflowID, time);
}

auto Activity::Mail(
    const identifier::Nym& nym,
    const Identifier& id,
    const StorageBox& box) const noexcept -> std::unique_ptr<Message>
{
    return imp_->Mail(nym, id, box);
}

auto Activity::Mail(
    const identifier::Nym& nym,
    const Message& mail,
    const StorageBox box,
    const PeerObject& text) const noexcept -> std::string
{
    return imp_->Mail(nym, mail, box, [&]() -> std::string {
        if (auto& out = text.Message(); out) {

            return *out;
        } else {

            return {};
        }
    }());
}

auto Activity::Mail(
    const identifier::Nym& nym,
    const Message& mail,
    const StorageBox box,
    const std::string& text) const noexcept -> std::string
{
    return imp_->Mail(nym, mail, box, text);
}

auto Activity::Mail(const identifier::Nym& nym, const StorageBox box)
    const noexcept -> ObjectList
{
    return imp_->Mail(nym, box);
}

auto Activity::MailRemove(
    const identifier::Nym& nym,
    const Identifier& id,
    const StorageBox box) const noexcept -> bool
{
    return imp_->MailRemove(nym, id, box);
}

auto Activity::MailText(
    const identifier::Nym& nym,
    const Identifier& id,
    const StorageBox& box,
    const PasswordPrompt& reason) const noexcept
    -> std::shared_future<std::string>
{
    return imp_->MailText(nym, id, box, reason);
}

auto Activity::MarkRead(
    const identifier::Nym& nymId,
    const Identifier& threadId,
    const Identifier& itemId) const noexcept -> bool
{
    return imp_->MarkRead(nymId, threadId, itemId);
}

auto Activity::MarkUnread(
    const identifier::Nym& nymId,
    const Identifier& threadId,
    const Identifier& itemId) const noexcept -> bool
{
    return imp_->MarkUnread(nymId, threadId, itemId);
}

auto Activity::Cheque(
    const identifier::Nym& nym,
    const std::string& id,
    const std::string& workflow) const noexcept -> ChequeData
{
    return imp_->Cheque(nym, id, workflow);
}

auto Activity::Transfer(
    const identifier::Nym& nym,
    const std::string& id,
    const std::string& workflow) const noexcept -> TransferData
{
    return imp_->Transfer(nym, id, workflow);
}

auto Activity::PaymentText(
    const identifier::Nym& nym,
    const std::string& id,
    const std::string& workflow) const noexcept
    -> std::shared_ptr<const std::string>
{
    return imp_->PaymentText(nym, id, workflow);
}

auto Activity::PreloadActivity(
    const identifier::Nym& nymID,
    const std::size_t count,
    const PasswordPrompt& reason) const noexcept -> void
{
    return imp_->PreloadActivity(nymID, count, reason);
}

auto Activity::PreloadThread(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    const std::size_t start,
    const std::size_t count,
    const PasswordPrompt& reason) const noexcept -> void
{
    return imp_->PreloadThread(nymID, threadID, start, count, reason);
}

auto Activity::Thread(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    proto::StorageThread& serialized) const noexcept -> bool
{
    return imp_->Thread(nymID, threadID, serialized);
}

auto Activity::Thread(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    AllocateOutput output) const noexcept -> bool
{
    return imp_->Thread(nymID, threadID, output);
}

auto Activity::Threads(const identifier::Nym& nym, const bool unreadOnly)
    const noexcept -> ObjectList
{
    return imp_->Threads(nym, unreadOnly);
}

auto Activity::UnreadCount(const identifier::Nym& nym) const noexcept
    -> std::size_t
{
    return imp_->UnreadCount(nym);
}

auto Activity::ThreadPublisher(const identifier::Nym& nym) const noexcept
    -> std::string
{
    return imp_->ThreadPublisher(nym);
}

Activity::~Activity() = default;
}  // namespace opentxs::api::client

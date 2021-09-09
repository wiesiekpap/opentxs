// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "api/client/activity/MailCache.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain;
class Contacts;
}  // namespace client

class Core;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class StorageThread;
}  // namespace proto

class Contact;
}  // namespace opentxs

namespace opentxs::api::client
{
struct Activity::Imp final : Lockable {
public:
    auto AddBlockchainTransaction(
        const Blockchain& api,
        const BlockchainTransaction& transaction) const noexcept -> bool;
    auto AddPaymentEvent(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const StorageBox type,
        const Identifier& itemID,
        const Identifier& workflowID,
        Time time) const noexcept -> bool;
    auto Mail(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box) const noexcept -> std::unique_ptr<Message>
    {
        return mail_.LoadMail(nym, id, box);
    }
    auto Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const StorageBox box,
        const std::string& text) const noexcept -> std::string;
    auto Mail(const identifier::Nym& nym, const StorageBox box) const noexcept
        -> ObjectList;
    auto MailRemove(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box) const noexcept -> bool;
    auto MailText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box,
        const PasswordPrompt& reason) const noexcept
        -> std::shared_future<std::string>
    {
        return mail_.GetText(nym, id, box, reason);
    }
    auto MarkRead(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const noexcept -> bool;
    auto MarkUnread(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const noexcept -> bool;
    auto Cheque(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept -> ChequeData;
    auto Transfer(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept -> TransferData;
    auto PaymentText(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept
        -> std::shared_ptr<const std::string>;
    auto PreloadActivity(
        const identifier::Nym& nymID,
        const std::size_t count,
        const PasswordPrompt& reason) const noexcept -> void;
    auto PreloadThread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const std::size_t start,
        const std::size_t count,
        const PasswordPrompt& reason) const noexcept -> void;
    auto Thread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        proto::StorageThread& serialzied) const noexcept -> bool;
    auto Thread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        AllocateOutput output) const noexcept -> bool;
    auto Threads(const identifier::Nym& nym, const bool unreadOnly = false)
        const noexcept -> ObjectList;
    auto UnreadCount(const identifier::Nym& nym) const noexcept -> std::size_t;
    auto ThreadPublisher(const identifier::Nym& nym) const noexcept
        -> std::string;

    Imp(const api::Core& api, const client::Contacts& contact) noexcept;

    ~Imp() final;

private:
    const api::Core& api_;
    const client::Contacts& contact_;
    const OTZMQPublishSocket message_loaded_;
    mutable activity::MailCache mail_;
    mutable std::mutex publisher_lock_;
    mutable std::map<OTIdentifier, OTZMQPublishSocket> thread_publishers_;
    mutable std::map<OTNymID, OTZMQPublishSocket> blockchain_publishers_;

    auto activity_preload_thread(
        OTPasswordPrompt reason,
        const OTIdentifier nymID,
        const std::size_t count) const noexcept -> void;
    auto thread_preload_thread(
        OTPasswordPrompt reason,
        const std::string nymID,
        const std::string threadID,
        const std::size_t start,
        const std::size_t count) const noexcept -> void;

#if OT_BLOCKCHAIN
    auto add_blockchain_transaction(
        const eLock& lock,
        const Blockchain& blockchain,
        const identifier::Nym& nym,
        const BlockchainTransaction& transaction) const noexcept -> bool;
#endif  // OT_BLOCKCHAIN
    auto nym_to_contact(const std::string& nymID) const noexcept
        -> std::shared_ptr<const Contact>;
#if OT_BLOCKCHAIN
    auto get_blockchain(const eLock&, const identifier::Nym& nymID)
        const noexcept -> const opentxs::network::zeromq::socket::Publish&;
#endif  // OT_BLOCKCHAIN
    auto get_publisher(const identifier::Nym& nymID) const noexcept
        -> const opentxs::network::zeromq::socket::Publish&;
    auto get_publisher(const identifier::Nym& nymID, std::string& endpoint)
        const noexcept -> const opentxs::network::zeromq::socket::Publish&;
    auto publish(const identifier::Nym& nymID, const Identifier& threadID)
        const noexcept -> void;
    auto start_publisher(const std::string& endpoint) const noexcept
        -> OTZMQPublishSocket;
    auto verify_thread_exists(const std::string& nym, const std::string& thread)
        const noexcept -> bool;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::api::client

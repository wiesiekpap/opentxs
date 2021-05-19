// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "internal/api/client/Client.hpp"
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

namespace internal
{
struct Core;
}  // namespace internal
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

namespace opentxs::api::client::implementation
{
class Activity final : virtual public api::client::internal::Activity, Lockable
{
public:
#if OT_BLOCKCHAIN
    auto AddBlockchainTransaction(
        const Blockchain& api,
        const BlockchainTransaction& transaction) const noexcept -> bool final;
#endif  // OT_BLOCKCHAIN
    auto AddPaymentEvent(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const StorageBox type,
        const Identifier& itemID,
        const Identifier& workflowID,
        Time time) const noexcept -> bool final;
    auto Mail(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box) const noexcept -> std::unique_ptr<Message> final;
    auto Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const StorageBox box,
        const PasswordPrompt& reason) const noexcept -> std::string final;
    auto Mail(const identifier::Nym& nym, const StorageBox box) const noexcept
        -> ObjectList final;
    auto MailRemove(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box) const noexcept -> bool final;
    auto MailText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box,
        const PasswordPrompt& reason) const noexcept
        -> std::shared_ptr<const std::string> final;
    auto MarkRead(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const noexcept -> bool final;
    auto MarkUnread(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const noexcept -> bool final;
    auto Cheque(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept -> ChequeData final;
    auto Transfer(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept -> TransferData final;
    auto PaymentText(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept
        -> std::shared_ptr<const std::string> final;
    void PreloadActivity(
        const identifier::Nym& nymID,
        const std::size_t count,
        const PasswordPrompt& reason) const noexcept final;
    void PreloadThread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const std::size_t start,
        const std::size_t count,
        const PasswordPrompt& reason) const noexcept final;
    auto Thread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        proto::StorageThread& serialzied) const noexcept -> bool final;
    auto Thread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        AllocateOutput output) const noexcept -> bool final;
    auto Threads(const identifier::Nym& nym, const bool unreadOnly = false)
        const noexcept -> ObjectList final;
    auto UnreadCount(const identifier::Nym& nym) const noexcept
        -> std::size_t final;
    auto ThreadPublisher(const identifier::Nym& nym) const noexcept
        -> std::string final;

    Activity(
        const api::internal::Core& api,
        const client::Contacts& contact) noexcept;

    ~Activity() final = default;

private:
    using MailCache =
        std::map<OTIdentifier, std::shared_ptr<const std::string>>;

    const api::internal::Core& api_;
    const client::Contacts& contact_;
    mutable std::mutex mail_cache_lock_;
    mutable MailCache mail_cache_;
    mutable std::mutex publisher_lock_;
    mutable std::map<OTIdentifier, OTZMQPublishSocket> thread_publishers_;
#if OT_BLOCKCHAIN
    mutable std::map<OTNymID, OTZMQPublishSocket> blockchain_publishers_;
#endif  // OT_BLOCKCHAIN

    void activity_preload_thread(
        OTPasswordPrompt reason,
        const OTIdentifier nymID,
        const std::size_t count) const noexcept;
    void preload(
        OTPasswordPrompt reason,
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box) const noexcept;
    void thread_preload_thread(
        OTPasswordPrompt reason,
        const std::string nymID,
        const std::string threadID,
        const std::size_t start,
        const std::size_t count) const noexcept;

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
    void publish(const identifier::Nym& nymID, const Identifier& threadID)
        const noexcept;
    auto start_publisher(const std::string& endpoint) const noexcept
        -> OTZMQPublishSocket;
    auto verify_thread_exists(const std::string& nym, const std::string& thread)
        const noexcept -> bool;

    Activity() = delete;
    Activity(const Activity&) = delete;
    Activity(Activity&&) = delete;
    auto operator=(const Activity&) -> Activity& = delete;
    auto operator=(Activity&&) -> Activity& = delete;
};
}  // namespace opentxs::api::client::implementation

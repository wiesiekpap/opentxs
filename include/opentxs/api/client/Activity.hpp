// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_ACTIVITY_HPP
#define OPENTXS_API_CLIENT_ACTIVITY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>
#include <string>
#include <tuple>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain;
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace proto
{
class StorageThread;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace client
{
class OPENTXS_EXPORT Activity
{
public:
    using ChequeData =
        std::pair<std::unique_ptr<const opentxs::Cheque>, OTUnitDefinition>;
    using TransferData =
        std::pair<std::unique_ptr<const opentxs::Item>, OTUnitDefinition>;
    using BlockchainTransaction =
        opentxs::blockchain::block::bitcoin::Transaction;

#if OT_BLOCKCHAIN
    virtual bool AddBlockchainTransaction(
        const Blockchain& api,
        const BlockchainTransaction& transaction) const noexcept = 0;
#endif  // OT_BLOCKCHAIN
    virtual bool AddPaymentEvent(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const StorageBox type,
        const Identifier& itemID,
        const Identifier& workflowID,
        Time time) const noexcept = 0;
    /**   Load a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual std::unique_ptr<Message> Mail(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box) const noexcept = 0;
    /**   Store a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    virtual std::string Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const StorageBox box,
        const PasswordPrompt& reason) const noexcept = 0;
    /**   Obtain a list of mail objects in a specified box
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] box the box to be listed
     */
    virtual ObjectList Mail(const identifier::Nym& nym, const StorageBox box)
        const noexcept = 0;
    /**   Delete a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    virtual bool MailRemove(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box) const noexcept = 0;
    /**   Retrieve the text from a message
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual std::shared_ptr<const std::string> MailText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box,
        const PasswordPrompt& reason) const noexcept = 0;
    /**   Mark a thread item as read
     *
     *    \param[in] nymId the identifier of the nym who owns the thread
     *    \param[in] threadId the thread containing the item to be marked
     *    \param[in] itemId the identifier of the item to be marked read
     *    \returns False if the nym, thread, or item does not exist
     */
    virtual bool MarkRead(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const noexcept = 0;
    /**   Mark a thread item as unread
     *
     *    \param[in] nymId the identifier of the nym who owns the thread
     *    \param[in] threadId the thread containing the item to be marked
     *    \param[in] itemId the identifier of the item to be marked unread
     *    \returns False if the nym, thread, or item does not exist
     */
    virtual bool MarkUnread(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const noexcept = 0;

    virtual ChequeData Cheque(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept = 0;

    virtual TransferData Transfer(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept = 0;

    /**   Summarize a payment workflow event in human-friendly test form
     *
     *    \param[in] nym the identifier of the nym who owns the thread
     *    \param[in] id the identifier of the payment item
     *    \param[in] workflow the identifier of the payment workflow
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual std::shared_ptr<const std::string> PaymentText(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept = 0;

    /**   Asynchronously cache the most recent items in each of a nym's threads
     *
     *    \param[in] nymID the identifier of the nym who owns the thread
     *    \param[in] count the number of items to preload in each thread
     */
    virtual void PreloadActivity(
        const identifier::Nym& nymID,
        const std::size_t count,
        const PasswordPrompt& reason) const noexcept = 0;
    /**   Asynchronously cache the items in an activity thread
     *
     *    \param[in] nymID the identifier of the nym who owns the thread
     *    \param[in] threadID the thread containing the items to be cached
     *    \param[in] start the first item to be cached
     *    \param[in] count the number of items to cache
     */
    virtual void PreloadThread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const std::size_t start,
        const std::size_t count,
        const PasswordPrompt& reason) const noexcept = 0;
    OPENTXS_NO_EXPORT virtual std::shared_ptr<proto::StorageThread> Thread(
        const identifier::Nym& nymID,
        const Identifier& threadID) const noexcept = 0;
    /**   Obtain a list of thread ids for the specified nym
     *
     *    \param[in] nym the identifier of the nym
     *    \param[in] unreadOnly if true, only return threads with unread items
     */
    virtual ObjectList Threads(
        const identifier::Nym& nym,
        const bool unreadOnly = false) const noexcept = 0;
    /**   Return the total number of unread thread items for a nym
     *
     *    \param[in] nymId
     */
    virtual std::size_t UnreadCount(
        const identifier::Nym& nym) const noexcept = 0;

    /** Activity thread update notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  ActivityThreadUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     */
    virtual std::string ThreadPublisher(
        const identifier::Nym& nym) const noexcept = 0;

    OPENTXS_NO_EXPORT virtual ~Activity() = default;

protected:
    Activity() = default;

private:
    Activity(const Activity&) = delete;
    Activity(Activity&&) = delete;
    Activity& operator=(const Activity&) = delete;
    Activity& operator=(Activity&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif

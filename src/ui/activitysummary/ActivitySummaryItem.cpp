// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/activitysummary/ActivitySummaryItem.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/base/Widget.hpp"

#define GET_TEXT_MILLISECONDS 10

#define OT_METHOD "opentxs::ui::implementation::ActivitySummaryItem::"

namespace opentxs::factory
{
auto ActivitySummaryItem(
    const ui::implementation::ActivitySummaryInternalInterface& parent,
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivitySummaryRowID& rowID,
    const ui::implementation::ActivitySummarySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const Flag& running) noexcept
    -> std::shared_ptr<ui::implementation::ActivitySummaryRowInternal>
{
    using ReturnType = ui::implementation::ActivitySummaryItem;

    return std::make_shared<ReturnType>(
        parent,
        api,
        nymID,
        rowID,
        sortKey,
        custom,
        running,
        ReturnType::LoadItemText(api, nymID, custom));
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ActivitySummaryItem::ActivitySummaryItem(
    const ActivitySummaryInternalInterface& parent,
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const ActivitySummaryRowID& rowID,
    const ActivitySummarySortKey& sortKey,
    CustomData& custom,
    const Flag& running,
    std::string text) noexcept
    : ActivitySummaryItemRow(parent, api, rowID, true)
    , running_(running)
    , nym_id_(nymID)
    , key_(sortKey)
    , display_name_(std::get<1>(key_))
    , text_(text)
    , type_(extract_custom<StorageBox>(custom, 1))
    , time_(extract_custom<Time>(custom, 3))
    , newest_item_thread_(nullptr)
    , newest_item_()
    , next_task_id_(0)
    , break_(false)
{
    startup(custom);
    newest_item_thread_.reset(
        new std::thread(&ActivitySummaryItem::get_text, this));

    OT_ASSERT(newest_item_thread_)
}

auto ActivitySummaryItem::DisplayName() const noexcept -> std::string
{
    sLock lock(shared_lock_);

    if (display_name_.empty()) { return api_.Contacts().ContactName(row_id_); }

    return display_name_;
}

auto ActivitySummaryItem::find_text(
    const PasswordPrompt& reason,
    const ItemLocator& locator) const noexcept -> std::string
{
    const auto& [itemID, box, accountID, thread] = locator;

    switch (box) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            auto text = api_.Activity().MailText(
                nym_id_, Identifier::Factory(itemID), box, reason);
            // TODO activity summary should subscribe for updates instead of
            // waiting for decryption

            return text.get();
        }
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            auto text = api_.Activity().PaymentText(nym_id_, itemID, accountID);

            if (text) {

                return *text;
            } else {
                LogOutput(OT_METHOD)(__func__)(": Cheque item does not exist.")
                    .Flush();
            }
        } break;
        case StorageBox::BLOCKCHAIN: {
            return api_.Blockchain().ActivityDescription(
                nym_id_, thread, itemID);
        }
        default: {
            OT_FAIL
        }
    }

    return {};
}

void ActivitySummaryItem::get_text() noexcept
{
    auto reason = api_.Factory().PasswordPrompt(__func__);
    eLock lock(shared_lock_, std::defer_lock);
    auto locator = ItemLocator{"", {}, "", api_.Factory().Identifier()};

    while (running_) {
        if (break_.load()) { return; }

        int taskID{0};

        if (newest_item_.Pop(taskID, locator)) {
            const auto text = find_text(reason, locator);
            lock.lock();
            text_ = text;
            lock.unlock();
            UpdateNotify();
        }

        Sleep(std::chrono::milliseconds(GET_TEXT_MILLISECONDS));
    }
}

auto ActivitySummaryItem::ImageURI() const noexcept -> std::string
{
    // TODO

    return {};
}

auto ActivitySummaryItem::LoadItemText(
    const api::client::Manager& api,
    const identifier::Nym& nym,
    const CustomData& custom) noexcept -> std::string
{
    const auto& box = *static_cast<const StorageBox*>(custom.at(1));
    const auto& thread = *static_cast<const OTIdentifier*>(custom.at(4));
    const auto& itemID = *static_cast<const std::string*>(custom.at(0));

    if (StorageBox::BLOCKCHAIN == box) {
        return api.Blockchain().ActivityDescription(nym, thread, itemID);
    }

    return {};
}

auto ActivitySummaryItem::reindex(
    const ActivitySummarySortKey& key,
    CustomData& custom) noexcept -> bool
{
    eLock lock(shared_lock_);
    key_ = key;
    lock.unlock();
    startup(custom);

    return true;
}

void ActivitySummaryItem::startup(CustomData& custom) noexcept
{
    auto locator = ItemLocator{
        extract_custom<std::string>(custom, 0),
        type_,
        extract_custom<std::string>(custom, 2),
        extract_custom<OTIdentifier>(custom, 4)};
    newest_item_.Push(++next_task_id_, std::move(locator));
}

auto ActivitySummaryItem::Text() const noexcept -> std::string
{
    sLock lock(shared_lock_);

    return text_;
}

auto ActivitySummaryItem::ThreadID() const noexcept -> std::string
{
    return row_id_->str();
}

auto ActivitySummaryItem::Timestamp() const noexcept -> Time
{
    sLock lock(shared_lock_);

    return time_;
}

auto ActivitySummaryItem::Type() const noexcept -> StorageBox
{
    sLock lock(shared_lock_);

    return type_;
}

ActivitySummaryItem::~ActivitySummaryItem()
{
    break_.store(true);

    if (newest_item_thread_ && newest_item_thread_->joinable()) {
        newest_item_thread_->join();
        newest_item_thread_.reset();
    }
}
}  // namespace opentxs::ui::implementation

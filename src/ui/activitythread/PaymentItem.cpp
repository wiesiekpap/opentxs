// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "ui/activitythread/PaymentItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <type_traits>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "ui/activitythread/ActivityThreadItem.hpp"
#include "ui/base/Widget.hpp"

#define OT_METHOD "opentxs::ui::implementation::PaymentItem::"

namespace opentxs::factory
{
auto PaymentItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>
{
    using ReturnType = ui::implementation::PaymentItem;
    auto [amount, display, memo, contract] =
        ReturnType::extract(api, nymID, rowID, custom);

    return std::make_shared<ReturnType>(
        parent,
        api,
        nymID,
        rowID,
        sortKey,
        custom,
        amount,
        std::move(display),
        std::move(memo),
        std::move(contract));
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
PaymentItem::PaymentItem(
    const ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    CustomData& custom,
    opentxs::Amount amount,
    std::string&& display,
    std::string&& memo,
    std::shared_ptr<const OTPayment>&& contract) noexcept
    : ActivityThreadItem(parent, api, nymID, rowID, sortKey, custom)
    , amount_(amount)
    , display_amount_(std::move(display))
    , memo_(std::move(memo))
    , payment_(std::move(contract))
{
    OT_ASSERT(false == nym_id_.empty())
    OT_ASSERT(false == item_id_.empty())
}

auto PaymentItem::Amount() const noexcept -> opentxs::Amount
{
    auto lock = sLock{shared_lock_};

    return amount_;
}

auto PaymentItem::Deposit() const noexcept -> bool
{
    switch (box_) {
        case StorageBox::INCOMINGCHEQUE: {
        } break;
        case StorageBox::OUTGOINGCHEQUE:
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::BLOCKCHAIN:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {

            return false;
        }
    }

    auto lock = sLock{shared_lock_};

    if (false == bool(payment_)) {
        LogOutput(OT_METHOD)(__func__)(": Payment not loaded.").Flush();

        return false;
    }

    auto task = api_.OTX().DepositPayment(nym_id_, payment_);

    if (0 == task.first) {
        LogOutput(OT_METHOD)(__func__)(": Failed to queue deposit.").Flush();

        return false;
    }

    return true;
}

auto PaymentItem::DisplayAmount() const noexcept -> std::string
{
    auto lock = sLock{shared_lock_};

    return display_amount_;
}

auto PaymentItem::extract(
    const api::client::Manager& api,
    const identifier::Nym& nym,
    const ActivityThreadRowID& row,
    CustomData& custom) noexcept
    -> std::tuple<
        opentxs::Amount,
        std::string,
        std::string,
        std::shared_ptr<const OTPayment>>
{
    const auto reason =
        api.Factory().PasswordPrompt("Decrypting payment activity");
    const auto& [itemID, box, account] = row;
    auto& text = *static_cast<std::string*>(custom.front());
    auto output = std::tuple<
        opentxs::Amount,
        std::string,
        std::string,
        std::shared_ptr<OTPayment>>{};
    auto& [amount, displayAmount, memo, payment] = output;

    switch (box) {
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            auto message =
                api.Activity().PaymentText(nym, itemID->str(), account->str());

            if (message) { text = *message; }

            const auto [cheque, contract] =
                api.Activity().Cheque(nym, itemID->str(), account->str());

            if (cheque) {
                memo = cheque->GetMemo().Get();
                amount = cheque->GetAmount();

                if (0 < contract->Version()) {
                    contract->FormatAmountLocale(
                        amount, displayAmount, ",", ".");
                }

                payment = api.Factory().Payment(String::Factory(*cheque));

                OT_ASSERT(payment);

                payment->SetTempValues(reason);
            }
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::BLOCKCHAIN:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
            OT_FAIL
        }
    }

    return std::move(output);
}

auto PaymentItem::Memo() const noexcept -> std::string
{
    auto lock = sLock{shared_lock_};

    return memo_;
}

auto PaymentItem::reindex(
    const ActivityThreadSortKey& key,
    CustomData& custom) noexcept -> bool
{
    auto [amount, display, memo, contract] =
        extract(Widget::api_, nym_id_, row_id_, custom);
    auto changed = ActivityThreadItem::reindex(key, custom);
    auto lock = eLock{shared_lock_};

    if (amount_ != amount) {
        amount_ = amount;
        changed = true;
    }

    if (display_amount_ != display) {
        display_amount_ = std::move(display);
        changed = true;
    }

    if (memo_ != memo) {
        memo_ = std::move(memo);
        changed = true;
    }

    if (contract && (!payment_)) {
        payment_ = std::move(contract);
        changed = true;
    }

    return changed;
}

PaymentItem::~PaymentItem() = default;
}  // namespace opentxs::ui::implementation

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "ui/accountactivity/AccountActivity.hpp"  // IWYU pragma: associated
#include "ui/accountactivity/BalanceItem.hpp"      // IWYU pragma: associated
#include "ui/accountactivity/BlockchainAccountActivity.hpp"  // IWYU pragma: associated
#include "ui/accountlist/AccountListItem.hpp"  // IWYU pragma: associated
#include "ui/accountlist/BlockchainAccountListItem.hpp"  // IWYU pragma: associated
#include "ui/accountsummary/AccountSummaryItem.hpp"  // IWYU pragma: associated
#include "ui/accountsummary/IssuerItem.hpp"          // IWYU pragma: associated
#include "ui/activitysummary/ActivitySummaryItem.hpp"  // IWYU pragma: associated
#include "ui/activitythread/ActivityThreadItem.hpp"  // IWYU pragma: associated
#include "ui/blockchainaccountstatus/BlockchainAccountStatus.hpp"  // IWYU pragma: associated
#include "ui/blockchainaccountstatus/BlockchainSubaccount.hpp"  // IWYU pragma: associated
#include "ui/blockchainaccountstatus/BlockchainSubaccountSource.hpp"  // IWYU pragma: associated
#include "ui/blockchainaccountstatus/BlockchainSubchain.hpp"  // IWYU pragma: associated
#include "ui/blockchainselection/BlockchainSelectionItem.hpp"  // IWYU pragma: associated
#include "ui/blockchainstatistics/BlockchainStatisticsItem.hpp"  // IWYU pragma: associated
#include "ui/contactlist/ContactListItem.hpp"  // IWYU pragma: associated
#include "ui/payablelist/PayableListItem.hpp"  // IWYU pragma: associated
#include "ui/unitlist/UnitListItem.hpp"        // IWYU pragma: associated

#include "opentxs/core/Log.hpp"
#include "ui/qt/SendMonitor.hpp"

namespace opentxs::ui::implementation
{
auto AccountActivity::DisplayScaleQt() noexcept -> ui::DisplayScaleQt&
{
    OT_FAIL;
}

auto AccountActivity::AmountValidator() noexcept -> ui::AmountValidator&
{
    OT_FAIL;
}

auto AccountActivity::DestinationValidator() noexcept
    -> ui::DestinationValidator&
{
    OT_FAIL;
}

auto AccountActivity::init_qt() noexcept -> void {}

auto AccountActivity::shutdown_qt() noexcept -> void {}

auto AccountActivity::SendMonitor() const noexcept
    -> implementation::SendMonitor&
{
    OT_FAIL;
}

auto AccountActivity::SendMonitor() noexcept -> implementation::SendMonitor&
{
    OT_FAIL;
}

auto AccountListItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto AccountSummaryItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto ActivitySummaryItem::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto ActivityThreadItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto BalanceItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto BlockchainAccountActivity::Send(
    const std::string&,
    const std::string&,
    const std::string&,
    Scale,
    SendMonitor::Callback) const noexcept -> int
{
    return -1;
}

auto BlockchainAccountListItem::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto BlockchainSelectionItem::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto BlockchainStatisticsItem::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto BlockchainSubaccount::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto BlockchainSubaccountSource::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto BlockchainSubchain::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto ContactListItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto IssuerItem::qt_data(const int, const int, QVariant&) const noexcept -> void
{
}

auto PayableListItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto UnitListItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::internal
{
auto List::MakeQT(const api::Core&) noexcept -> ui::qt::internal::Model*
{
    return nullptr;
}
}  // namespace opentxs::ui::internal

namespace opentxs::ui::qt::internal
{
auto Model::ChangeRow(ui::internal::Row*, ui::internal::Row*) noexcept -> void
{
}

auto Model::DeleteRow(ui::internal::Row*) noexcept -> void {}

auto Model::InsertRow(
    ui::internal::Row*,
    ui::internal::Row*,
    std::shared_ptr<ui::internal::Row>) noexcept -> void
{
}

auto Model::MoveRow(
    ui::internal::Row*,
    ui::internal::Row*,
    ui::internal::Row*) noexcept -> void
{
}

Model::~Model() = default;
}  // namespace opentxs::ui::qt::internal

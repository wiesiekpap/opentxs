// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTSUMMARYITEM_HPP
#define OPENTXS_UI_ACCOUNTSUMMARYITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "ListRow.hpp"
#include "opentxs/SharedPimpl.hpp"

namespace opentxs
{
namespace ui
{
class AccountSummaryItem;
}  // namespace ui

using OTUIAccountSummaryItem = SharedPimpl<ui::AccountSummaryItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT AccountSummaryItem : virtual public ListRow
{
public:
    virtual auto AccountID() const noexcept -> std::string = 0;
    virtual auto Balance() const noexcept -> Amount = 0;
    virtual auto DisplayBalance() const noexcept -> std::string = 0;
    virtual auto Name() const noexcept -> std::string = 0;

    ~AccountSummaryItem() override = default;

protected:
    AccountSummaryItem() noexcept = default;

private:
    AccountSummaryItem(const AccountSummaryItem&) = delete;
    AccountSummaryItem(AccountSummaryItem&&) = delete;
    auto operator=(const AccountSummaryItem&) -> AccountSummaryItem& = delete;
    auto operator=(AccountSummaryItem&&) -> AccountSummaryItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

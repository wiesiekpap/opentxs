// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTLIST_HPP
#define OPENTXS_UI_ACCOUNTLIST_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIAccountList) opentxs::ui::AccountList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class AccountList;
class AccountListItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class AccountList : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountListItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountListItem>
    Next() const noexcept = 0;

    OPENTXS_EXPORT ~AccountList() override = default;

protected:
    AccountList() noexcept = default;

private:
    AccountList(const AccountList&) = delete;
    AccountList(AccountList&&) = delete;
    AccountList& operator=(const AccountList&) = delete;
    AccountList& operator=(AccountList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

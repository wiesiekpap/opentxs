// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTSUMMARY_HPP
#define OPENTXS_UI_ACCOUNTSUMMARY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

namespace opentxs
{
namespace ui
{
class AccountSummary;
class IssuerItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT AccountSummary : virtual public List
{
public:
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::IssuerItem> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::IssuerItem> = 0;

    ~AccountSummary() override = default;

protected:
    AccountSummary() noexcept = default;

private:
    AccountSummary(const AccountSummary&) = delete;
    AccountSummary(AccountSummary&&) = delete;
    auto operator=(const AccountSummary&) -> AccountSummary& = delete;
    auto operator=(AccountSummary&&) -> AccountSummary& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

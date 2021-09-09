// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTLISTITEM_HPP
#define OPENTXS_UI_ACCOUNTLISTITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "ListRow.hpp"
#include "opentxs/SharedPimpl.hpp"

namespace opentxs
{
namespace ui
{
class AccountListItem;
}  // namespace ui

using OTUIAccountListItem = SharedPimpl<ui::AccountListItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT AccountListItem : virtual public ListRow
{
public:
    virtual auto AccountID() const noexcept -> std::string = 0;
    virtual auto Balance() const noexcept -> Amount = 0;
    virtual auto ContractID() const noexcept -> std::string = 0;
    virtual auto DisplayBalance() const noexcept -> std::string = 0;
    virtual auto DisplayUnit() const noexcept -> std::string = 0;
    virtual auto Name() const noexcept -> std::string = 0;
    virtual auto NotaryID() const noexcept -> std::string = 0;
    virtual auto NotaryName() const noexcept -> std::string = 0;
    virtual auto Type() const noexcept -> AccountType = 0;
    virtual auto Unit() const noexcept -> contact::ContactItemType = 0;

    ~AccountListItem() override = default;

protected:
    AccountListItem() noexcept = default;

private:
    AccountListItem(const AccountListItem&) = delete;
    AccountListItem(AccountListItem&&) = delete;
    auto operator=(const AccountListItem&) -> AccountListItem& = delete;
    auto operator=(AccountListItem&&) -> AccountListItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

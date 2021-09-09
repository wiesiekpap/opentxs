// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PAYABLELISTITEM_HPP
#define OPENTXS_UI_PAYABLELISTITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "ContactListItem.hpp"
#include "opentxs/SharedPimpl.hpp"

namespace opentxs
{
namespace ui
{
class PayableListItem;
}  // namespace ui

using OTUIPayableListItem = SharedPimpl<ui::PayableListItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT PayableListItem : virtual public ContactListItem
{
public:
    virtual auto PaymentCode() const noexcept -> std::string = 0;

    ~PayableListItem() override = default;

protected:
    PayableListItem() noexcept = default;

private:
    PayableListItem(const PayableListItem&) = delete;
    PayableListItem(PayableListItem&&) = delete;
    auto operator=(const PayableListItem&) -> PayableListItem& = delete;
    auto operator=(PayableListItem&&) -> PayableListItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

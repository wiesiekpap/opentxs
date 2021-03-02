// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PAYABLELIST_HPP
#define OPENTXS_UI_PAYABLELIST_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIPayableList) opentxs::ui::PayableList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class PayableList;
class PayableListItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class PayableList : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::PayableListItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::PayableListItem>
    Next() const noexcept = 0;

    OPENTXS_EXPORT ~PayableList() override = default;

protected:
    PayableList() noexcept = default;

private:
    PayableList(const PayableList&) = delete;
    PayableList(PayableList&&) = delete;
    PayableList& operator=(const PayableList&) = delete;
    PayableList& operator=(PayableList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_UNITLIST_HPP
#define OPENTXS_UI_UNITLIST_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIUnitList) opentxs::ui::UnitList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class UnitList;
class UnitListItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class UnitList : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::UnitListItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::UnitListItem>
    Next() const noexcept = 0;

    ~UnitList() override = default;

protected:
    UnitList() noexcept = default;

private:
    UnitList(const UnitList&) = delete;
    UnitList(UnitList&&) = delete;
    UnitList& operator=(const UnitList&) = delete;
    UnitList& operator=(UnitList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "ui/unitlist/UnitListItem.hpp"  // IWYU pragma: associated

#if OT_QT
#include <QObject>
#endif  // OT_QT
#include <memory>

#if OT_QT
#include "opentxs/ui/qt/UnitList.hpp"
#endif  // OT_QT

// #define OT_METHOD "opentxs::ui::implementation::UnitListItem::"

namespace opentxs::factory
{
auto UnitListItem(
    const ui::implementation::UnitListInternalInterface& parent,
    const api::client::internal::Manager& api,
    const ui::implementation::UnitListRowID& rowID,
    const ui::implementation::UnitListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::UnitListRowInternal>
{
    using ReturnType = ui::implementation::UnitListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
UnitListItem::UnitListItem(
    const UnitListInternalInterface& parent,
    const api::client::internal::Manager& api,
    const UnitListRowID& rowID,
    const UnitListSortKey& sortKey,
    [[maybe_unused]] CustomData& custom) noexcept
    : UnitListItemRow(parent, api, rowID, true)
    , name_(sortKey)
{
}

#if OT_QT
auto UnitListItem::qt_data(const int column, int role) const noexcept
    -> QVariant
{
    switch (role) {
        case UnitListQt::UnitIDRole: {
            return static_cast<unsigned int>(Unit());
        }
        case Qt::DisplayRole: {
            return Name().c_str();
        }
        default: {
        }
    }
    return {};
}
#endif
}  // namespace opentxs::ui::implementation

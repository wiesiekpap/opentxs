// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/ui/UnitListItem.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "ui/base/Row.hpp"

class QVariant;

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class UnitListItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using UnitListItemRow =
    Row<UnitListRowInternal, UnitListInternalInterface, UnitListRowID>;

class UnitListItem final : public UnitListItemRow
{
public:
    auto Name() const noexcept -> std::string final { return name_; }
    auto Unit() const noexcept -> core::UnitType final { return row_id_; }

    UnitListItem(
        const UnitListInternalInterface& parent,
        const api::session::Client& api,
        const UnitListRowID& rowID,
        const UnitListSortKey& sortKey,
        CustomData& custom) noexcept;

    ~UnitListItem() final = default;

private:
    const UnitListSortKey name_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const UnitListSortKey&, CustomData&) noexcept -> bool final
    {
        return false;
    }

    UnitListItem() = delete;
    UnitListItem(const UnitListItem&) = delete;
    UnitListItem(UnitListItem&&) = delete;
    auto operator=(const UnitListItem&) -> UnitListItem& = delete;
    auto operator=(UnitListItem&&) -> UnitListItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::UnitListItem>;

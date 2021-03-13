// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "ui/activitythread/PendingSend.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/activitythread/ActivityThreadItem.hpp"
#include "ui/base/Widget.hpp"

//#define OT_METHOD "opentxs::ui::implementation::PendingSend::"

namespace opentxs::factory
{
auto PendingSend(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>
{
    using ReturnType = ui::implementation::PendingSend;

    return std::make_shared<ReturnType>(
        parent, api, nymID, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
PendingSend::PendingSend(
    const ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    CustomData& custom) noexcept
    : ActivityThreadItem(
          parent,
          api,
          nymID,
          rowID,
          sortKey,
          custom,
          false,
          true)
    , amount_(extract_custom<opentxs::Amount>(custom, 1))
    , display_amount_(extract_custom<std::string>(custom, 2))
    , memo_(extract_custom<std::string>(custom, 3))
{
    OT_ASSERT(false == nym_id_.empty())
    OT_ASSERT(false == item_id_.empty())
}
}  // namespace opentxs::ui::implementation

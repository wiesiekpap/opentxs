// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYSUMMARYITEM_HPP
#define OPENTXS_UI_ACTIVITYSUMMARYITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <string>

#include "ListRow.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace ui
{
class ActivitySummaryItem;
}  // namespace ui

using OTUIActivitySummaryItem = SharedPimpl<ui::ActivitySummaryItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ActivitySummaryItem : virtual public ListRow
{
public:
    virtual auto DisplayName() const noexcept -> std::string = 0;
    virtual auto ImageURI() const noexcept -> std::string = 0;
    virtual auto Text() const noexcept -> std::string = 0;
    virtual auto ThreadID() const noexcept -> std::string = 0;
    virtual auto Timestamp() const noexcept -> Time = 0;
    virtual auto Type() const noexcept -> StorageBox = 0;

    ~ActivitySummaryItem() override = default;

protected:
    ActivitySummaryItem() noexcept = default;

private:
    ActivitySummaryItem(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem(ActivitySummaryItem&&) = delete;
    auto operator=(const ActivitySummaryItem&) -> ActivitySummaryItem& = delete;
    auto operator=(ActivitySummaryItem&&) -> ActivitySummaryItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

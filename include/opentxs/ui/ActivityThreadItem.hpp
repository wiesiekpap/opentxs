// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYTHREADITEM_HPP
#define OPENTXS_UI_ACTIVITYTHREADITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <string>

#include "ListRow.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace ui
{
class ActivityThreadItem;
}  // namespace ui

using OTUIActivityThreadItem = SharedPimpl<ui::ActivityThreadItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ActivityThreadItem : virtual public ListRow
{
public:
    virtual auto Amount() const noexcept -> opentxs::Amount = 0;
    virtual auto Deposit() const noexcept -> bool = 0;
    virtual auto DisplayAmount() const noexcept -> std::string = 0;
    virtual auto From() const noexcept -> std::string = 0;
    virtual auto Loading() const noexcept -> bool = 0;
    virtual auto MarkRead() const noexcept -> bool = 0;
    virtual auto Memo() const noexcept -> std::string = 0;
    virtual auto Outgoing() const noexcept -> bool = 0;
    virtual auto Pending() const noexcept -> bool = 0;
    virtual auto Text() const noexcept -> std::string = 0;
    virtual auto Timestamp() const noexcept -> Time = 0;
    virtual auto Type() const noexcept -> StorageBox = 0;

    ~ActivityThreadItem() override = default;

protected:
    ActivityThreadItem() noexcept = default;

private:
    ActivityThreadItem(const ActivityThreadItem&) = delete;
    ActivityThreadItem(ActivityThreadItem&&) = delete;
    auto operator=(const ActivityThreadItem&) -> ActivityThreadItem& = delete;
    auto operator=(ActivityThreadItem&&) -> ActivityThreadItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

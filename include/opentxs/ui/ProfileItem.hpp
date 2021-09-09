// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PROFILEITEM_HPP
#define OPENTXS_UI_PROFILEITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "ListRow.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace ui
{
class ProfileItem;
}  // namespace ui

using OTUIProfileItem = SharedPimpl<ui::ProfileItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ProfileItem : virtual public ListRow
{
public:
    virtual auto ClaimID() const noexcept -> std::string = 0;
    virtual auto Delete() const noexcept -> bool = 0;
    virtual auto IsActive() const noexcept -> bool = 0;
    virtual auto IsPrimary() const noexcept -> bool = 0;
    virtual auto SetActive(const bool& active) const noexcept -> bool = 0;
    virtual auto SetPrimary(const bool& primary) const noexcept -> bool = 0;
    virtual auto SetValue(const std::string& value) const noexcept -> bool = 0;
    virtual auto Value() const noexcept -> std::string = 0;

    ~ProfileItem() override = default;

protected:
    ProfileItem() noexcept = default;

private:
    ProfileItem(const ProfileItem&) = delete;
    ProfileItem(ProfileItem&&) = delete;
    auto operator=(const ProfileItem&) -> ProfileItem& = delete;
    auto operator=(ProfileItem&&) -> ProfileItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

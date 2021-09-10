// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PROFILESUBSECTION_HPP
#define OPENTXS_UI_PROFILESUBSECTION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/ui/ListRow.hpp"

namespace opentxs
{
namespace ui
{
class ProfileSubsection;
}  // namespace ui

using OTUIProfileSubsection = SharedPimpl<ui::ProfileSubsection>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ProfileSubsection : virtual public List,
                                         virtual public ListRow
{
public:
    virtual auto AddItem(
        const std::string& value,
        const bool primary,
        const bool active) const noexcept -> bool = 0;
    virtual auto Delete(const std::string& claimID) const noexcept -> bool = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileItem> = 0;
    virtual auto Name(const std::string& lang) const noexcept
        -> std::string = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileItem> = 0;
    virtual auto SetActive(const std::string& claimID, const bool active)
        const noexcept -> bool = 0;
    virtual auto SetPrimary(const std::string& claimID, const bool primary)
        const noexcept -> bool = 0;
    virtual auto SetValue(const std::string& claimID, const std::string& value)
        const noexcept -> bool = 0;
    virtual auto Type() const noexcept -> contact::ContactItemType = 0;

    ~ProfileSubsection() override = default;

protected:
    ProfileSubsection() noexcept = default;

private:
    ProfileSubsection(const ProfileSubsection&) = delete;
    ProfileSubsection(ProfileSubsection&&) = delete;
    auto operator=(const ProfileSubsection&) -> ProfileSubsection& = delete;
    auto operator=(ProfileSubsection&&) -> ProfileSubsection& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

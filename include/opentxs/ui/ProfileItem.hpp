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

#ifdef SWIG
// clang-format off
%template(OTUIProfileItem) opentxs::SharedPimpl<opentxs::ui::ProfileItem>;
%rename(UIProfileItem) opentxs::ui::ProfileItem;
// clang-format on
#endif  // SWIG

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
class ProfileItem : virtual public ListRow
{
public:
    OPENTXS_EXPORT virtual std::string ClaimID() const noexcept = 0;
    OPENTXS_EXPORT virtual bool Delete() const noexcept = 0;
    OPENTXS_EXPORT virtual bool IsActive() const noexcept = 0;
    OPENTXS_EXPORT virtual bool IsPrimary() const noexcept = 0;
    OPENTXS_EXPORT virtual bool SetActive(
        const bool& active) const noexcept = 0;
    OPENTXS_EXPORT virtual bool SetPrimary(
        const bool& primary) const noexcept = 0;
    OPENTXS_EXPORT virtual bool SetValue(
        const std::string& value) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Value() const noexcept = 0;

    OPENTXS_EXPORT ~ProfileItem() override = default;

protected:
    ProfileItem() noexcept = default;

private:
    ProfileItem(const ProfileItem&) = delete;
    ProfileItem(ProfileItem&&) = delete;
    ProfileItem& operator=(const ProfileItem&) = delete;
    ProfileItem& operator=(ProfileItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

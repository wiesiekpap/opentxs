// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTLISTITEM_HPP
#define OPENTXS_UI_CONTACTLISTITEM_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <string>

#include "ListRow.hpp"
#include "opentxs/SharedPimpl.hpp"

#ifdef SWIG
// clang-format off
%template(OTUIContactListItem) opentxs::SharedPimpl<opentxs::ui::ContactListItem>;
%rename(UIContactListItem) opentxs::ui::ContactListItem;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ContactListItem;
}  // namespace ui

using OTUIContactListItem = SharedPimpl<ui::ContactListItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class ContactListItem : virtual public ListRow
{
public:
    OPENTXS_EXPORT virtual std::string ContactID() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string DisplayName() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string ImageURI() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Section() const noexcept = 0;

    OPENTXS_EXPORT ~ContactListItem() override = default;

protected:
    ContactListItem() noexcept = default;

private:
    ContactListItem(const ContactListItem&) = delete;
    ContactListItem(ContactListItem&&) = delete;
    ContactListItem& operator=(const ContactListItem&) = delete;
    ContactListItem& operator=(ContactListItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

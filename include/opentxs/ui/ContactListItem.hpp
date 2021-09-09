// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTLISTITEM_HPP
#define OPENTXS_UI_CONTACTLISTITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "ListRow.hpp"
#include "opentxs/SharedPimpl.hpp"

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
class OPENTXS_EXPORT ContactListItem : virtual public ListRow
{
public:
    virtual auto ContactID() const noexcept -> std::string = 0;
    virtual auto DisplayName() const noexcept -> std::string = 0;
    virtual auto ImageURI() const noexcept -> std::string = 0;
    virtual auto Section() const noexcept -> std::string = 0;

    ~ContactListItem() override = default;

protected:
    ContactListItem() noexcept = default;

private:
    ContactListItem(const ContactListItem&) = delete;
    ContactListItem(ContactListItem&&) = delete;
    auto operator=(const ContactListItem&) -> ContactListItem& = delete;
    auto operator=(ContactListItem&&) -> ContactListItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

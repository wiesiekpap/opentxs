// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTLIST_HPP
#define OPENTXS_UI_CONTACTLIST_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

namespace opentxs
{
namespace ui
{
class ContactList;
class ContactListItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ContactList : virtual public List
{
public:
    virtual auto AddContact(
        const std::string& label,
        const std::string& paymentCode = "",
        const std::string& nymID = "") const noexcept -> std::string = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactListItem> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactListItem> = 0;

    OPENTXS_NO_EXPORT ~ContactList() override = default;

protected:
    ContactList() noexcept = default;

private:
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    auto operator=(const ContactList&) -> ContactList& = delete;
    auto operator=(ContactList&&) -> ContactList& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

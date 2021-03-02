// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_MESSAGABLELIST_HPP
#define OPENTXS_UI_MESSAGABLELIST_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIMessagableList) opentxs::ui::MessagableList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ContactListItem;
class MessagableList;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class MessagableList : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactListItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactListItem>
    Next() const noexcept = 0;

    OPENTXS_EXPORT ~MessagableList() override = default;

protected:
    MessagableList() noexcept = default;

private:
    MessagableList(const MessagableList&) = delete;
    MessagableList(MessagableList&&) = delete;
    MessagableList& operator=(const MessagableList&) = delete;
    MessagableList& operator=(MessagableList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

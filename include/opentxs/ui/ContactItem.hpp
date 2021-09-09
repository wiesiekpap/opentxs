// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTITEM_HPP
#define OPENTXS_UI_CONTACTITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "ListRow.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace ui
{
class ContactItem;
}  // namespace ui

using OTUIContactItem = SharedPimpl<ui::ContactItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ContactItem : virtual public ListRow
{
public:
    virtual auto ClaimID() const noexcept -> std::string = 0;
    virtual auto IsActive() const noexcept -> bool = 0;
    virtual auto IsPrimary() const noexcept -> bool = 0;
    virtual auto Value() const noexcept -> std::string = 0;

    ~ContactItem() override = default;

protected:
    ContactItem() noexcept = default;

private:
    ContactItem(const ContactItem&) = delete;
    ContactItem(ContactItem&&) = delete;
    auto operator=(const ContactItem&) -> ContactItem& = delete;
    auto operator=(ContactItem&&) -> ContactItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

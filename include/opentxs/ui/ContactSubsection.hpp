// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTSUBSECTION_HPP
#define OPENTXS_UI_CONTACTSUBSECTION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/ui/ListRow.hpp"

namespace opentxs
{
namespace ui
{
class ContactSubsection;
}  // namespace ui

using OTUIContactSubsection = SharedPimpl<ui::ContactSubsection>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ContactSubsection : virtual public List,
                                         virtual public ListRow
{
public:
    virtual auto Name(const std::string& lang) const noexcept
        -> std::string = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactItem> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactItem> = 0;
    virtual auto Type() const noexcept -> contact::ContactItemType = 0;

    ~ContactSubsection() override = default;

protected:
    ContactSubsection() noexcept = default;

private:
    ContactSubsection(const ContactSubsection&) = delete;
    ContactSubsection(ContactSubsection&&) = delete;
    auto operator=(const ContactSubsection&) -> ContactSubsection& = delete;
    auto operator=(ContactSubsection&&) -> ContactSubsection& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

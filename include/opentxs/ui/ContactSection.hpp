// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTSECTION_HPP
#define OPENTXS_UI_CONTACTSECTION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/ui/ListRow.hpp"

namespace opentxs
{
namespace ui
{
class ContactSection;
class ContactSubsection;
}  // namespace ui

using OTUIContactSection = SharedPimpl<ui::ContactSection>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ContactSection : virtual public List,
                                      virtual public ListRow
{
public:
    virtual auto Name(const std::string& lang) const noexcept
        -> std::string = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactSubsection> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactSubsection> = 0;
    virtual auto Type() const noexcept -> contact::ContactSectionName = 0;

    ~ContactSection() override = default;

protected:
    ContactSection() noexcept = default;

private:
    ContactSection(const ContactSection&) = delete;
    ContactSection(ContactSection&&) = delete;
    auto operator=(const ContactSection&) -> ContactSection& = delete;
    auto operator=(ContactSection&&) -> ContactSection& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

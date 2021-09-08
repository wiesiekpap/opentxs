// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ISSUERITEM_HPP
#define OPENTXS_UI_ISSUERITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/ui/ListRow.hpp"

#ifdef SWIG
// clang-format off
%template(OTUIIssuerItem) opentxs::SharedPimpl<opentxs::ui::IssuerItem>;
%rename(UIIssuerItem) opentxs::ui::IssuerItem;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class AccountSummaryItem;
class IssuerItem;
}  // namespace ui

using OTUIIssuerItem = SharedPimpl<ui::IssuerItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class IssuerItem : virtual public List, virtual public ListRow
{
public:
    OPENTXS_EXPORT virtual bool ConnectionState() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Debug() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Name() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem>
    Next() const noexcept = 0;
    OPENTXS_EXPORT virtual bool Trusted() const noexcept = 0;

    OPENTXS_EXPORT ~IssuerItem() override = default;

protected:
    IssuerItem() noexcept = default;

private:
    IssuerItem(const IssuerItem&) = delete;
    IssuerItem(IssuerItem&&) = delete;
    IssuerItem& operator=(const IssuerItem&) = delete;
    IssuerItem& operator=(IssuerItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

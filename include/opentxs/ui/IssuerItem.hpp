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
class OPENTXS_EXPORT IssuerItem : virtual public List, virtual public ListRow
{
public:
    virtual auto ConnectionState() const noexcept -> bool = 0;
    virtual auto Debug() const noexcept -> std::string = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem> = 0;
    virtual auto Name() const noexcept -> std::string = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem> = 0;
    virtual auto Trusted() const noexcept -> bool = 0;

    ~IssuerItem() override = default;

protected:
    IssuerItem() noexcept = default;

private:
    IssuerItem(const IssuerItem&) = delete;
    IssuerItem(IssuerItem&&) = delete;
    auto operator=(const IssuerItem&) -> IssuerItem& = delete;
    auto operator=(IssuerItem&&) -> IssuerItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_WIDGET_HPP
#define OPENTXS_UI_WIDGET_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Identifier.hpp"

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT Widget
{
public:
    virtual void ClearCallbacks() const noexcept = 0;
    virtual void SetCallback(SimpleCallback cb) const noexcept = 0;
    virtual auto WidgetID() const noexcept -> OTIdentifier = 0;

    virtual ~Widget() = default;

protected:
    Widget() noexcept = default;

private:
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    auto operator=(const Widget&) -> Widget& = delete;
    auto operator=(Widget&&) -> Widget& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

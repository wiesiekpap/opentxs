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

#ifdef SWIG
// clang-format off
%extend opentxs::ui::Widget {
    std::string WidgetID() const noexcept
    {
        return $self->WidgetID()->str();
    }
}
%ignore opentxs::ui::Widget::SetCallback;
%ignore opentxs::ui::Widget::WidgetID;
%rename(UIWidget) opentxs::ui::Widget;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class Widget
{
public:
    OPENTXS_EXPORT virtual void ClearCallbacks() const noexcept = 0;
    OPENTXS_EXPORT virtual void SetCallback(
        SimpleCallback cb) const noexcept = 0;
    OPENTXS_EXPORT virtual OTIdentifier WidgetID() const noexcept = 0;

    OPENTXS_EXPORT virtual ~Widget() = default;

protected:
    Widget() noexcept = default;

private:
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

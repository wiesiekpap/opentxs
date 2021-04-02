// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_LIST_HPP
#define OPENTXS_UI_LIST_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cassert>
#include <limits>

#include "opentxs/Types.hpp"
#include "opentxs/ui/Widget.hpp"

#ifdef SWIG
// clang-format off
%rename(UIList) opentxs::ui::List;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class List : virtual public Widget
{
public:
    OPENTXS_EXPORT ~List() override = default;

protected:
    List() noexcept = default;

private:
    List(const List&) = delete;
    List(List&&) = delete;
    List& operator=(const List&) = delete;
    List& operator=(List&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

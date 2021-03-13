// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_LISTROW_HPP
#define OPENTXS_UI_LISTROW_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/ui/Widget.hpp"

#ifdef SWIG
// clang-format off
%rename(UIListRow) opentxs::ui::ListRow;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ListRow : virtual public Widget
{
public:
    OPENTXS_EXPORT virtual bool Last() const noexcept = 0;
    OPENTXS_EXPORT virtual bool Valid() const noexcept = 0;

    OPENTXS_EXPORT ~ListRow() override = default;

protected:
    ListRow() noexcept = default;

private:
    ListRow(const ListRow&) = delete;
    ListRow(ListRow&&) = delete;
    ListRow& operator=(const ListRow&) = delete;
    ListRow& operator=(ListRow&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

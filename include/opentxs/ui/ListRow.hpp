// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_LISTROW_HPP
#define OPENTXS_UI_LISTROW_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/ui/Widget.hpp"

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ListRow : virtual public Widget
{
public:
    virtual auto Last() const noexcept -> bool = 0;
    virtual auto Valid() const noexcept -> bool = 0;

    ~ListRow() override = default;

protected:
    ListRow() noexcept = default;

private:
    ListRow(const ListRow&) = delete;
    ListRow(ListRow&&) = delete;
    auto operator=(const ListRow&) -> ListRow& = delete;
    auto operator=(ListRow&&) -> ListRow& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

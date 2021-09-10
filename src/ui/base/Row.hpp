// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/Lockable.hpp"
#include "ui/base/RowType.hpp"
#include "ui/base/Widget.hpp"

namespace opentxs::ui::implementation
{
template <typename InterfaceType, typename ParentType, typename IdentifierType>
class Row : public RowType<InterfaceType, ParentType, IdentifierType>,
            public Widget,
            public Lockable
{
public:
    auto AddChildren(implementation::CustomData&& data) noexcept -> void final
    {
    }

protected:
    Row(const ParentType& parent,
        const api::client::Manager& api,
        const IdentifierType id,
        const bool valid) noexcept
        : RowType<InterfaceType, ParentType, IdentifierType>(parent, id, valid)
        , Widget(api, parent.WidgetID())
    {
    }
    Row() = delete;
    Row(const Row&) = delete;
    Row(Row&&) = delete;
    auto operator=(const Row&) -> Row& = delete;
    auto operator=(Row&&) -> Row& = delete;

    ~Row() override = default;
};
}  // namespace opentxs::ui::implementation

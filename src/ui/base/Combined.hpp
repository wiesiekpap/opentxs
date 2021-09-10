// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ui/base/List.hpp"
#include "ui/base/RowType.hpp"

namespace opentxs::ui::implementation
{
template <typename ListTemplate, typename RowTemplate, typename SortKey>
class Combined : public ListTemplate, public RowTemplate
{
public:
    auto AddChildren(implementation::CustomData&& data) noexcept -> void final
    {
        ListTemplate::AddChildrenToList(std::move(data));
    }

protected:
    SortKey key_;

    Combined(
        const api::client::Manager& api,
        const typename ListTemplate::ListPrimaryID::interface_type& primaryID,
        const Identifier& widgetID,
        const typename RowTemplate::RowParentType& parent,
        const typename RowTemplate::RowIdentifierType id,
        const SortKey key,
        const bool reverseSort = false) noexcept
        : ListTemplate(
              api,
              primaryID,
              widgetID,
              reverseSort,
              true,
              {},
              parent.GetQt())
        , RowTemplate(parent, id, true)
        , key_(key)
    {
    }

    ~Combined() override = default;

private:
    auto qt_parent() noexcept -> internal::Row* final { return this; }

    Combined() = delete;
    Combined(const Combined&) = delete;
    Combined(Combined&&) = delete;
    auto operator=(const Combined&) -> Combined& = delete;
    auto operator=(Combined&&) -> Combined& = delete;
};
}  // namespace opentxs::ui::implementation

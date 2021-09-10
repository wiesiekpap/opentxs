// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/blockchainaccountstatus/BlockchainSubaccountSource.hpp"  // IWYU pragma: associated

#include <memory>

#include "ui/base/List.hpp"

// #define OT_METHOD "opentxs::ui::implementation::BlockchainSubaccountSource::"

namespace opentxs::factory
{
auto BlockchainSubaccountSourceWidget(
    const ui::implementation::BlockchainAccountStatusInternalInterface& parent,
    const api::client::Manager& api,
    const ui::implementation::BlockchainAccountStatusRowID& rowID,
    const ui::implementation::BlockchainAccountStatusSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::BlockchainAccountStatusRowInternal>
{
    using ReturnType = ui::implementation::BlockchainSubaccountSource;

    return std::make_unique<ReturnType>(parent, api, rowID, key, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainSubaccountSource::BlockchainSubaccountSource(
    const BlockchainAccountStatusInternalInterface& parent,
    const api::client::Manager& api,
    const BlockchainAccountStatusRowID& rowID,
    const BlockchainAccountStatusSortKey& key,
    CustomData& custom) noexcept
    : Combined(
          api,
          parent.Owner(),
          parent.WidgetID(),
          parent,
          rowID,
          key,
          false)
{
}

auto BlockchainSubaccountSource::construct_row(
    const BlockchainSubaccountSourceRowID& id,
    const BlockchainSubaccountSourceSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::BlockchainSubaccountWidget(*this, api_, id, index, custom);
}

auto BlockchainSubaccountSource::reindex(
    const implementation::BlockchainAccountStatusSortKey& key,
    implementation::CustomData& custom) noexcept -> bool
{
    return false;
}

BlockchainSubaccountSource::~BlockchainSubaccountSource() = default;
}  // namespace opentxs::ui::implementation

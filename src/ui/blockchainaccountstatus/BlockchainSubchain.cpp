// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/blockchainaccountstatus/BlockchainSubchain.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "ui/base/Widget.hpp"

//#define OT_METHOD "opentxs::ui::implementation::BlockchainSubchain::"

namespace opentxs::factory
{
auto BlockchainSubchainWidget(
    const ui::implementation::BlockchainSubaccountInternalInterface& parent,
    const api::client::Manager& api,
    const ui::implementation::BlockchainSubaccountRowID& rowID,
    const ui::implementation::BlockchainSubaccountSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::BlockchainSubaccountRowInternal>
{
    using ReturnType = ui::implementation::BlockchainSubchain;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainSubchain::BlockchainSubchain(
    const BlockchainSubaccountInternalInterface& parent,
    const api::client::Manager& api,
    const BlockchainSubaccountRowID& rowID,
    const BlockchainSubaccountSortKey& sortKey,
    CustomData& custom) noexcept
    : BlockchainSubchainRow(parent, api, rowID, true)
    , name_(sortKey)
    , progress_(extract_custom<std::string>(custom, 0))
{
}

auto BlockchainSubchain::Name() const noexcept -> std::string
{
    auto lock = sLock{shared_lock_};

    return name_;
}

auto BlockchainSubchain::Progress() const noexcept -> std::string
{
    auto lock = sLock{shared_lock_};

    return progress_;
}

auto BlockchainSubchain::reindex(
    const BlockchainSubaccountSortKey& key,
    CustomData& custom) noexcept -> bool
{
    auto progress = extract_custom<std::string>(custom, 0);
    eLock lock{shared_lock_};
    auto changed{false};

    if (name_ != key) {
        changed = true;
        name_ = key;
    }

    if (progress_ != progress) {
        changed = true;
        progress_ = std::move(progress);
    }

    return changed;
}

BlockchainSubchain::~BlockchainSubchain() = default;
}  // namespace opentxs::ui::implementation

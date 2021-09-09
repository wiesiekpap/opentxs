// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "ui/accountsummary/AccountSummaryItem.hpp"  // IWYU pragma: associated

#include <atomic>
#include <memory>
#include <utility>

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "ui/base/Widget.hpp"

namespace opentxs::factory
{
auto AccountSummaryItem(
    const ui::implementation::IssuerItemInternalInterface& parent,
    const api::client::Manager& api,
    const ui::implementation::IssuerItemRowID& rowID,
    const ui::implementation::IssuerItemSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::IssuerItemRowInternal>
{
    using ReturnType = ui::implementation::AccountSummaryItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
AccountSummaryItem::AccountSummaryItem(
    const IssuerItemInternalInterface& parent,
    const api::client::Manager& api,
    const IssuerItemRowID& rowID,
    const IssuerItemSortKey& sortKey,
    CustomData& custom) noexcept
    : AccountSummaryItemRow(parent, api, rowID, true)
    , account_id_(std::get<0>(row_id_).get())
    , currency_(std::get<1>(row_id_))
    , balance_(extract_custom<Amount>(custom))
    , name_(sortKey)
    , contract_(load_unit(api_, account_id_))
{
}

auto AccountSummaryItem::DisplayBalance() const noexcept -> std::string
{
    if (0 == contract_->Version()) {
        eLock lock(shared_lock_);

        try {
            contract_ = api_.Wallet().UnitDefinition(
                api_.Storage().AccountContract(account_id_));
        } catch (...) {
        }
    }

    sLock lock(shared_lock_);

    if (0 < contract_->Version()) {
        const auto amount = balance_.load();
        std::string output{};
        const auto formatted =
            contract_->FormatAmountLocale(amount, output, ",", ".");

        if (formatted) { return output; }

        return std::to_string(amount);
    }

    return {};
}

auto AccountSummaryItem::load_unit(const api::Core& api, const Identifier& id)
    -> OTUnitDefinition
{
    try {
        return api.Wallet().UnitDefinition(api.Storage().AccountContract(id));
    } catch (...) {

        return api.Factory().UnitDefinition();
    }
}

auto AccountSummaryItem::Name() const noexcept -> std::string
{
    sLock lock(shared_lock_);

    return name_;
}

auto AccountSummaryItem::reindex(
    const IssuerItemSortKey& key,
    CustomData& custom) noexcept -> bool
{
    balance_.store(extract_custom<Amount>(custom));
    eLock lock(shared_lock_);
    name_ = key;

    return true;
}
}  // namespace opentxs::ui::implementation

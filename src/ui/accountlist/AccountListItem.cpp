// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "ui/accountlist/AccountListItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "ui/base/Widget.hpp"

// #define OT_METHOD "opentxs::ui::implementation::AccountListItem::"

namespace opentxs::factory
{
auto AccountListItem(
    const ui::implementation::AccountListInternalInterface& parent,
    const api::client::Manager& api,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountListRowInternal>
{
    using ReturnType = ui::implementation::AccountListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
AccountListItem::AccountListItem(
    const AccountListInternalInterface& parent,
    const api::client::Manager& api,
    const AccountListRowID& rowID,
    const AccountListSortKey& sortKey,
    CustomData& custom) noexcept
    : AccountListItemRow(parent, api, rowID, true)
    , type_(AccountType::Custodial)
    , unit_(sortKey.first)
    , contract_(load_unit(api_, extract_custom<OTUnitID>(custom, 2)))
    , notary_(load_server(api_, api.Factory().ServerID(sortKey.second)))
    , balance_(extract_custom<Amount>(custom, 1))
    , name_(extract_custom<std::string>(custom, 3))
{
}

auto AccountListItem::Balance() const noexcept -> Amount
{
    Lock lock{lock_};

    return balance_;
}

auto AccountListItem::DisplayBalance() const noexcept -> std::string
{
    auto output = std::string{};
    const auto formatted =
        contract_->FormatAmountLocale(balance_, output, ",", ".");

    if (formatted) { return output; }

    return std::to_string(balance_);
}

auto AccountListItem::load_server(
    const api::Core& api,
    const identifier::Server& id) -> OTServerContract
{
    try {

        return api.Wallet().Server(id);
    } catch (...) {

        return api.Factory().ServerContract();
    }
}

auto AccountListItem::load_unit(
    const api::Core& api,
    const identifier::UnitDefinition& id) -> OTUnitDefinition
{
    try {

        return api.Wallet().UnitDefinition(id);
    } catch (...) {

        return api.Factory().UnitDefinition();
    }
}

auto AccountListItem::Name() const noexcept -> std::string
{
    Lock lock{lock_};

    return name_;
}

auto AccountListItem::reindex(
    const AccountListSortKey& key,
    CustomData& custom) noexcept -> bool
{
    const auto blockchain = extract_custom<bool>(custom, 0);
    const auto balance = extract_custom<Amount>(custom, 1);
    const auto unitID = extract_custom<OTUnitID>(custom, 2);
    const auto name = extract_custom<std::string>(custom, 3);

    OT_ASSERT(false == blockchain);
    OT_ASSERT(contract_->ID() == unitID);

    Lock lock{lock_};
    auto output{false};

    if (balance_ != balance) {
        output = true;
        balance_ = balance;
    }

    if (name_ != name) {
        output = true;
        name_ = name;
    }

    return output;
}
}  // namespace opentxs::ui::implementation

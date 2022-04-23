// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
struct AccountTreeRow {
    ot::UnallocatedCString account_id_{};
    ot::UnallocatedCString contract_id_{};
    ot::UnallocatedCString display_unit_{};
    ot::UnallocatedCString name_{};
    ot::UnallocatedCString notary_id_{};
    ot::UnallocatedCString notary_name_{};
    ot::AccountType type_{};
    ot::UnitType unit_{};
    int polarity_{};
    ot::Amount balance_{};
    ot::UnallocatedCString display_balance_{};
};

struct AccountCurrencyData {
    ot::UnitType type_{};
    ot::UnallocatedCString name_{};
    ot::UnallocatedVector<AccountTreeRow> rows_{};
};

struct AccountTreeData {
    ot::UnallocatedVector<AccountCurrencyData> rows_{};
};

auto check_account_tree(
    const User& user,
    const AccountTreeData& expected) noexcept -> bool;
auto check_account_tree_qt(
    const User& user,
    const AccountTreeData& expected) noexcept -> bool;
auto init_account_tree(const User& user, Counter& counter) noexcept -> void;
auto print_account_tree(const User& user) noexcept -> ot::UnallocatedCString;
}  // namespace ottest

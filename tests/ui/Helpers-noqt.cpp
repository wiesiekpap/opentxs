// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

namespace ottest
{
auto check_account_activity_qt(
    const User&,
    const ot::Identifier&,
    const AccountActivityData&) noexcept -> bool
{
    return true;
}

auto check_activity_thread_qt(
    const User&,
    const ot::Identifier&,
    const ActivityThreadData&) noexcept -> bool
{
    return true;
}

auto check_blockchain_account_status_qt(
    const User&,
    const ot::blockchain::Type,
    const BlockchainAccountStatusData&) noexcept -> bool
{
    return true;
}

auto check_contact_list_qt(
    const User&,
    const std::vector<ContactListData>&) noexcept -> bool
{
    return true;
}
}  // namespace ottest

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/rpc/AccountType.hpp"

#ifndef OPENTXS_RPC_ACCOUNT_DATA_HPP
#define OPENTXS_RPC_ACCOUNT_DATA_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/rpc/Types.hpp"

namespace opentxs
{
namespace proto
{
class AccountData;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace rpc
{
class OPENTXS_EXPORT AccountData
{
public:
    auto ConfirmedBalance() const noexcept -> Amount;
    auto ConfirmedBalance_str() const noexcept -> std::string;
    auto ID() const noexcept -> const std::string&;
    auto Issuer() const noexcept -> const std::string&;
    auto Name() const noexcept -> const std::string&;
    auto Owner() const noexcept -> const std::string&;
    auto PendingBalance() const noexcept -> Amount;
    auto PendingBalance_str() const noexcept -> std::string;
    OPENTXS_NO_EXPORT auto Serialize(proto::AccountData& dest) const noexcept
        -> bool;
    auto Type() const noexcept -> AccountType;
    auto Unit() const noexcept -> const std::string&;

    OPENTXS_NO_EXPORT AccountData(
        const proto::AccountData& serialized) noexcept(false);
    OPENTXS_NO_EXPORT AccountData(
        const std::string& id,
        const std::string& name,
        const std::string& unit,
        const std::string& owner,
        const std::string& issuer,
        const std::string& balanceS,
        const std::string& pendingS,
        Amount balance,
        Amount pending,
        AccountType type) noexcept(false);
    OPENTXS_NO_EXPORT AccountData(const AccountData&) noexcept;
    OPENTXS_NO_EXPORT AccountData(AccountData&&) noexcept;

    OPENTXS_NO_EXPORT ~AccountData();

private:
    struct Imp;

    Imp* imp_;

    AccountData() noexcept;
    auto operator=(const AccountData&) -> AccountData& = delete;
    auto operator=(AccountData&&) -> AccountData& = delete;
};
}  // namespace rpc
}  // namespace opentxs
#endif

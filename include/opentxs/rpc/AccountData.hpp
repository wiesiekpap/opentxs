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
class AccountData
{
public:
    OPENTXS_EXPORT auto ConfirmedBalance() const noexcept -> Amount;
    OPENTXS_EXPORT auto ID() const noexcept -> const std::string&;
    OPENTXS_EXPORT auto Issuer() const noexcept -> const std::string&;
    OPENTXS_EXPORT auto Name() const noexcept -> const std::string&;
    OPENTXS_EXPORT auto Owner() const noexcept -> const std::string&;
    OPENTXS_EXPORT auto PendingBalance() const noexcept -> Amount;
    auto Serialize(proto::AccountData& dest) const noexcept -> bool;
    OPENTXS_EXPORT auto Type() const noexcept -> AccountType;
    OPENTXS_EXPORT auto Unit() const noexcept -> const std::string&;

    AccountData(const proto::AccountData& serialized) noexcept(false);
    AccountData(
        const std::string& id,
        const std::string& name,
        const std::string& unit,
        const std::string& owner,
        const std::string& issuer,
        Amount balance,
        Amount pending,
        AccountType type) noexcept(false);
    AccountData(const AccountData&) noexcept;
    AccountData(AccountData&&) noexcept;

    ~AccountData();

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

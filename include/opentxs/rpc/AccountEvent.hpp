// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/rpc/AccountEventType.hpp"

#ifndef OPENTXS_RPC_ACCOUNT_EVENT_HPP
#define OPENTXS_RPC_ACCOUNT_EVENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/rpc/Types.hpp"

namespace opentxs
{
namespace proto
{
class AccountEvent;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace rpc
{
class OPENTXS_EXPORT AccountEvent
{
public:
    auto AccountID() const noexcept -> const std::string&;
    auto ConfirmedAmount() const noexcept -> Amount;
    auto ConfirmedAmount_str() const noexcept -> const std::string&;
    auto ContactID() const noexcept -> const std::string&;
    auto Memo() const noexcept -> const std::string&;
    auto PendingAmount() const noexcept -> Amount;
    auto PendingAmount_str() const noexcept -> const std::string&;
    OPENTXS_NO_EXPORT auto Serialize(proto::AccountEvent& dest) const noexcept
        -> bool;
    auto State() const noexcept -> int;
    auto Timestamp() const noexcept -> Time;
    auto Type() const noexcept -> AccountEventType;
    auto UUID() const noexcept -> const std::string&;
    auto WorkflowID() const noexcept -> const std::string&;

    OPENTXS_NO_EXPORT AccountEvent(
        const proto::AccountEvent& serialized) noexcept(false);
    OPENTXS_NO_EXPORT AccountEvent(
        const std::string& account,
        AccountEventType type,
        const std::string& contact,
        const std::string& workflow,
        const std::string& amountS,
        const std::string& pendingS,
        Amount amount,
        Amount pending,
        opentxs::Time time,
        const std::string& memo,
        const std::string& uuid,
        int state) noexcept(false);
    OPENTXS_NO_EXPORT AccountEvent(const AccountEvent&) noexcept;
    OPENTXS_NO_EXPORT AccountEvent(AccountEvent&&) noexcept;

    OPENTXS_NO_EXPORT ~AccountEvent();

private:
    struct Imp;

    Imp* imp_;

    AccountEvent() noexcept;
    auto operator=(const AccountEvent&) -> AccountEvent& = delete;
    auto operator=(AccountEvent&&) -> AccountEvent& = delete;
};
}  // namespace rpc
}  // namespace opentxs
#endif

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
class AccountEvent
{
public:
    OPENTXS_EXPORT auto AccountID() const noexcept -> const std::string&;
    OPENTXS_EXPORT auto ConfirmedAmount() const noexcept -> Amount;
    OPENTXS_EXPORT auto ContactID() const noexcept -> const std::string&;
    OPENTXS_EXPORT auto Memo() const noexcept -> const std::string&;
    OPENTXS_EXPORT auto PendingAmount() const noexcept -> Amount;
    auto Serialize(proto::AccountEvent& dest) const noexcept -> bool;
    OPENTXS_EXPORT auto State() const noexcept -> int;
    OPENTXS_EXPORT auto Timestamp() const noexcept -> Time;
    OPENTXS_EXPORT auto Type() const noexcept -> AccountEventType;
    OPENTXS_EXPORT auto UUID() const noexcept -> const std::string&;
    OPENTXS_EXPORT auto WorkflowID() const noexcept -> const std::string&;

    AccountEvent(const proto::AccountEvent& serialized) noexcept(false);
    AccountEvent(
        const std::string& account,
        AccountEventType type,
        const std::string& contact,
        const std::string& workflow,
        Amount amount,
        Amount pending,
        opentxs::Time time,
        const std::string& memo,
        const std::string& uuid,
        int state) noexcept(false);
    AccountEvent(const AccountEvent&) noexcept;
    AccountEvent(AccountEvent&&) noexcept;

    ~AccountEvent();

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

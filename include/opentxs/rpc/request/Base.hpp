// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/rpc/CommandType.hpp"

#ifndef OPENTXS_RPC_REQUEST_BASE_HPP
#define OPENTXS_RPC_REQUEST_BASE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/rpc/Types.hpp"

namespace opentxs
{
namespace proto
{
class RPCCommand;
}  // namespace proto

namespace rpc
{
namespace request
{
class Base;
class GetAccountActivity;
class GetAccountBalance;
class ListAccounts;
class ListNyms;
class SendPayment;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs

namespace opentxs
{
namespace rpc
{
namespace request
{
auto Factory(ReadView serialized) noexcept -> Base;
auto Factory(const proto::RPCCommand& serialized) noexcept -> Base;

class Base
{
public:
    using SessionIndex = int;
    using Identifiers = std::vector<std::string>;
    using AssociateNyms = Identifiers;

    struct Imp;

    OPENTXS_EXPORT auto asGetAccountActivity() const noexcept
        -> const GetAccountActivity&;
    OPENTXS_EXPORT auto asGetAccountBalance() const noexcept
        -> const GetAccountBalance&;
    OPENTXS_EXPORT auto asListAccounts() const noexcept -> const ListAccounts&;
    OPENTXS_EXPORT auto asListNyms() const noexcept -> const ListNyms&;
    OPENTXS_EXPORT auto asSendPayment() const noexcept -> const SendPayment&;

    OPENTXS_EXPORT auto AssociatedNyms() const noexcept -> const AssociateNyms&;
    OPENTXS_EXPORT auto Cookie() const noexcept -> const std::string&;
    OPENTXS_EXPORT auto Serialize(AllocateOutput dest) const noexcept -> bool;
    auto Serialize(proto::RPCCommand& dest) const noexcept -> bool;
    OPENTXS_EXPORT auto Session() const noexcept -> SessionIndex;
    OPENTXS_EXPORT auto Type() const noexcept -> CommandType;
    OPENTXS_EXPORT auto Version() const noexcept -> VersionNumber;

    OPENTXS_EXPORT Base() noexcept;
    OPENTXS_EXPORT Base(const Base&) noexcept;
    OPENTXS_EXPORT Base(Base&&) noexcept;
    OPENTXS_EXPORT auto operator=(const Base&) noexcept -> Base&;
    OPENTXS_EXPORT auto operator=(Base&&) noexcept -> Base&;

    OPENTXS_EXPORT virtual ~Base();

protected:
    Imp* imp_;

    Base(Imp* imp) noexcept;
};
}  // namespace request
}  // namespace rpc
}  // namespace opentxs
#endif

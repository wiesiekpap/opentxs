// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_RPC_RESPONSE_LIST_ACCOUNTS_HPP
#define OPENTXS_RPC_RESPONSE_LIST_ACCOUNTS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/rpc/response/Base.hpp"

namespace opentxs
{
namespace proto
{
class RPCResponse;
}  // namespace proto

namespace rpc
{
namespace request
{
class ListAccounts;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs

namespace opentxs
{
namespace rpc
{
namespace response
{
class OPENTXS_EXPORT ListAccounts final : public Base
{
public:
    auto AccountIDs() const noexcept -> const Identifiers&;

    /// throws std::runtime_error for invalid constructor arguments
    OPENTXS_NO_EXPORT ListAccounts(
        const request::ListAccounts& request,
        Responses&& response,
        Identifiers&& accounts) noexcept(false);
    OPENTXS_NO_EXPORT ListAccounts(
        const proto::RPCResponse& serialized) noexcept(false);
    ListAccounts() noexcept;

    ~ListAccounts() final;

private:
    ListAccounts(const ListAccounts&) = delete;
    ListAccounts(ListAccounts&&) = delete;
    auto operator=(const ListAccounts&) -> ListAccounts& = delete;
    auto operator=(ListAccounts&&) -> ListAccounts& = delete;
};
}  // namespace response
}  // namespace rpc
}  // namespace opentxs
#endif

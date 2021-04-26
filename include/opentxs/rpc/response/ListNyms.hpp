// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_RPC_RESPONSE_LIST_NYMS_HPP
#define OPENTXS_RPC_RESPONSE_LIST_NYMS_HPP

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
class ListNyms;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs

namespace opentxs
{
namespace rpc
{
namespace response
{
class OPENTXS_EXPORT ListNyms final : public Base
{
public:
    auto NymIDs() const noexcept -> const Identifiers&;

    /// throws std::runtime_error for invalid constructor arguments
    OPENTXS_NO_EXPORT ListNyms(
        const request::ListNyms& request,
        Responses&& response,
        Identifiers&& accounts) noexcept(false);
    OPENTXS_NO_EXPORT ListNyms(const proto::RPCResponse& serialized) noexcept(
        false);
    ListNyms() noexcept;

    ~ListNyms() final;

private:
    ListNyms(const ListNyms&) = delete;
    ListNyms(ListNyms&&) = delete;
    auto operator=(const ListNyms&) -> ListNyms& = delete;
    auto operator=(ListNyms&&) -> ListNyms& = delete;
};
}  // namespace response
}  // namespace rpc
}  // namespace opentxs
#endif

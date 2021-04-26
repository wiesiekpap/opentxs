// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_RPC_LIST_ACCOUNTS_HPP
#define OPENTXS_RPC_LIST_ACCOUNTS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/rpc/request/Base.hpp"

namespace opentxs
{
namespace proto
{
class RPCCommand;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace rpc
{
namespace request
{
class OPENTXS_EXPORT ListAccounts final : public Base
{
public:
    static auto DefaultVersion() noexcept -> VersionNumber;

    auto FilterNotary() const noexcept -> const std::string&;
    auto FilterNym() const noexcept -> const std::string&;
    auto FilterUnit() const noexcept -> const std::string&;

    /// throws std::runtime_error for invalid constructor arguments
    ListAccounts(
        SessionIndex session,
        const std::string& filterNym = {},
        const std::string& filterNotary = {},
        const std::string& filterUnit = {},
        const AssociateNyms& nyms = {}) noexcept(false);
    ListAccounts(const proto::RPCCommand& serialized) noexcept(false);
    ListAccounts() noexcept;

    ~ListAccounts() final;

private:
    ListAccounts(const ListAccounts&) = delete;
    ListAccounts(ListAccounts&&) = delete;
    auto operator=(const ListAccounts&) -> ListAccounts& = delete;
    auto operator=(ListAccounts&&) -> ListAccounts& = delete;
};
}  // namespace request
}  // namespace rpc
}  // namespace opentxs
#endif

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <tuple>

#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace contract
{
class Server;
}  // namespace contract

class Data;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace curve
{
class OPENTXS_EXPORT Client : virtual public socket::Socket
{
public:
    static auto RandomKeypair() noexcept
        -> std::pair<UnallocatedCString, UnallocatedCString>;

    virtual auto SetKeysZ85(
        const UnallocatedCString& serverPublic,
        const UnallocatedCString& clientPrivate,
        const UnallocatedCString& clientPublic) const noexcept -> bool = 0;
    virtual auto SetServerPubkey(
        const contract::Server& contract) const noexcept -> bool = 0;
    virtual auto SetServerPubkey(const Data& key) const noexcept -> bool = 0;

    ~Client() override = default;

protected:
    Client() noexcept = default;

private:
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    auto operator=(const Client&) -> Client& = delete;
    auto operator=(Client&&) -> Client& = delete;
};
}  // namespace curve
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

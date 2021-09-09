// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_CURVE_SERVER_HPP
#define OPENTXS_NETWORK_ZEROMQ_CURVE_SERVER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
class Secret;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace curve
{
class OPENTXS_EXPORT Server : virtual public socket::Socket
{
public:
    virtual auto SetDomain(const std::string& domain) const noexcept
        -> bool = 0;
    virtual auto SetPrivateKey(const Secret& key) const noexcept -> bool = 0;
    virtual auto SetPrivateKey(const std::string& z85) const noexcept
        -> bool = 0;

    ~Server() override = default;

protected:
    Server() noexcept = default;

private:
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;
};
}  // namespace curve
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

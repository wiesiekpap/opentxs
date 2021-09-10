// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_SOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_SOCKET_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <tuple>

#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class OPENTXS_EXPORT Socket
{
public:
    using SendResult = std::pair<opentxs::SendResult, OTZMQMessage>;
    enum class Direction : bool { Bind = false, Connect = true };

    virtual operator void*() const noexcept = 0;

    virtual auto Close() const noexcept -> bool = 0;
    virtual auto Context() const noexcept -> const zeromq::Context& = 0;
    virtual auto SetTimeouts(
        const std::chrono::milliseconds& linger,
        const std::chrono::milliseconds& send,
        const std::chrono::milliseconds& receive) const noexcept -> bool = 0;
    // Do not call Start during callback execution
    virtual auto Start(const std::string& endpoint) const noexcept -> bool = 0;
    // StartAsync version may be called during callback execution
    virtual void StartAsync(const std::string& endpoint) const noexcept = 0;
    virtual auto Type() const noexcept -> SocketType = 0;

    virtual ~Socket() = default;

protected:
    Socket() noexcept = default;

private:
    Socket(const Socket&) = delete;
    Socket(Socket&&) = default;
    auto operator=(const Socket&) -> Socket& = delete;
    auto operator=(Socket&&) -> Socket& = default;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

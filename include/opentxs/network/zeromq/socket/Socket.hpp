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

#ifdef SWIG
// clang-format off
%extend opentxs::network::zeromq::socket::Socket {
    bool SetTimeouts(
        const int& lingerMilliseconds,
        const int& sendMilliseconds,
        const int& receiveMilliseconds) const
    {
        return $self->SetTimeouts(
            std::chrono::milliseconds(lingerMilliseconds),
            std::chrono::milliseconds(sendMilliseconds),
            std::chrono::milliseconds(receiveMilliseconds));
    }
}
%ignore opentxs::network::zeromq::socket::Socket::Context;
%ignore opentxs::network::zeromq::socket::Socket::SetTimeouts;
%ignore opentxs::network::zeromq::socket::Socket::operator void*() const;
%template(ZMQMessageSendResult) std::pair<opentxs::SendResult, Pimpl<opentxs::network::zeromq::Message>>;
%rename(ZMQSocket) opentxs::network::zeromq::socket::Socket;
// clang-format on
#endif  // SWIG

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
class Socket
{
public:
    using SendResult = std::pair<opentxs::SendResult, OTZMQMessage>;
    enum class Direction : bool { Bind = false, Connect = true };

    OPENTXS_EXPORT virtual operator void*() const noexcept = 0;

    OPENTXS_EXPORT virtual bool Close() const noexcept = 0;
    OPENTXS_EXPORT virtual const zeromq::Context& Context() const noexcept = 0;
    OPENTXS_EXPORT virtual bool SetTimeouts(
        const std::chrono::milliseconds& linger,
        const std::chrono::milliseconds& send,
        const std::chrono::milliseconds& receive) const noexcept = 0;
    // Do not call Start during callback execution
    OPENTXS_EXPORT virtual bool Start(
        const std::string& endpoint) const noexcept = 0;
    // StartAsync version may be called during callback execution
    OPENTXS_EXPORT virtual void StartAsync(
        const std::string& endpoint) const noexcept = 0;
    OPENTXS_EXPORT virtual SocketType Type() const noexcept = 0;

    OPENTXS_EXPORT virtual ~Socket() = default;

protected:
    Socket() noexcept = default;

private:
    Socket(const Socket&) = delete;
    Socket(Socket&&) = default;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = default;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

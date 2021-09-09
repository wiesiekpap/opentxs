// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_REQUEST_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_REQUEST_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/curve/Client.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Request;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQRequestSocket = Pimpl<network::zeromq::socket::Request>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class OPENTXS_EXPORT Request : virtual public curve::Client
{
public:
    auto Send(opentxs::Pimpl<opentxs::network::zeromq::Message>& message)
        const noexcept -> std::pair<
            opentxs::SendResult,
            opentxs::Pimpl<opentxs::network::zeromq::Message>>
    {
        return send_request(message.get());
    }
    auto Send(Message& message) const noexcept -> std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>
    {
        return send_request(message);
    }
    template <typename Input>
    auto Send(const Input& data) const noexcept -> std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>;
    virtual auto SetSocksProxy(const std::string& proxy) const noexcept
        -> bool = 0;

    ~Request() override = default;

protected:
    Request() noexcept = default;

private:
    friend OTZMQRequestSocket;

    virtual auto clone() const noexcept -> Request* = 0;
    virtual auto send_request(Message& message) const noexcept
        -> SendResult = 0;

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

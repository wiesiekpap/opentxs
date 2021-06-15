// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Request final : virtual public zeromq::socket::Request,
                      public Socket,
                      public zeromq::curve::implementation::Client
{
public:
    auto SetSocksProxy(const std::string& proxy) const noexcept -> bool final;

    Request(const zeromq::Context& context) noexcept;

    ~Request() final;

private:
    auto clone() const noexcept -> Request* final;
    auto send_request(zeromq::Message& message) const noexcept
        -> SendResult final;
    auto wait(const Lock& lock) const noexcept -> bool;

    Request() = delete;
    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation

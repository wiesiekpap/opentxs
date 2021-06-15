// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "internal/api/Api.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"

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

namespace opentxs::api::implementation
{
class Log final : virtual public api::internal::Log
{
public:
    Log(const opentxs::network::zeromq::Context& zmq,
        const std::string& endpoint);

    ~Log() final = default;

private:
    OTZMQListenCallback callback_;
    OTZMQPullSocket socket_;
    OTZMQPublishSocket publish_socket_;
    const bool publish_;

    void callback(opentxs::network::zeromq::Message& message);
    void print(
        const int level,
        const std::string& text,
        const std::string& thread);
#ifdef ANDROID
    void print_android(
        const int level,
        const std::string& text,
        const std::string& thread);
#endif

    Log() = delete;
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    auto operator=(const Log&) -> Log& = delete;
    auto operator=(Log&&) -> Log& = delete;
};
}  // namespace opentxs::api::implementation

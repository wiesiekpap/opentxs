// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/Log.hpp"     // IWYU pragma: associated

#include <cstdlib>
#include <future>
#include <iostream>
#include <memory>
#include <string_view>
#include <utility>

#include "internal/api/Factory.hpp"
#include "internal/util/Log.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto Log(const zmq::Context& zmq, std::string_view endpoint) noexcept
    -> std::unique_ptr<api::internal::Log>
{
    using ReturnType = api::imp::Log;
    internal::Log::Start();

    return std::make_unique<ReturnType>(zmq, UnallocatedCString{endpoint});
}
}  // namespace opentxs::factory

namespace opentxs::api::imp
{
Log::Log(const zmq::Context& zmq, const UnallocatedCString endpoint)
    : callback_(opentxs::network::zeromq::ListenCallback::Factory(
          [&](auto&& msg) -> void { callback(std::move(msg)); }))
    , socket_(zmq.PullSocket(callback_, zmq::socket::Direction::Bind))
    , publish_socket_(zmq.PublishSocket())
    , publish_{!endpoint.empty()}
{
    const auto started = socket_->Start(opentxs::internal::Log::Endpoint());

    if (false == started) { abort(); }

    if (publish_) {
        const auto publishStarted = publish_socket_->Start(endpoint);
        if (false == publishStarted) { abort(); }
    }
}

auto Log::callback(zmq::Message&& message) noexcept -> void
{
    if (message.Body().size() < 3) { return; }

    const auto& levelFrame = message.Body_at(0);
    const auto text = UnallocatedCString{message.Body_at(1).Bytes()};
    const auto id = UnallocatedCString{message.Body_at(2).Bytes()};

    try {
        const auto level = levelFrame.as<int>();
        print(level, text, id);
    } catch (...) {
        std::cout << "Invalid level size: " << levelFrame.size() << '\n';

        OT_FAIL;
    }

    if (message.Body().size() >= 4) {
        const auto& promiseFrame = message.Body_at(3);
        auto* pPromise = promiseFrame.as<std::promise<void>*>();

        if (nullptr != pPromise) { pPromise->set_value(); }
    }

    if (publish_) { publish_socket_->Send(std::move(message)); }
}
}  // namespace opentxs::api::imp

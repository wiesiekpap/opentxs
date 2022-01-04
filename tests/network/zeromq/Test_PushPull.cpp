// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <chrono>
#include <ctime>
#include <string>

#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_PushPull : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};

    const std::string endpoint_{"inproc://opentxs/test/push_pull_test"};

    Test_PushPull()
        : context_(ot::Context().ZMQ())
    {
    }
};

TEST_F(Test_PushPull, Push_Pull)
{
    bool callbackFinished{false};

    auto pullCallback = zmq::ListenCallback::Factory(
        [this, &callbackFinished](auto&& input) -> void {
            EXPECT_EQ(1, input.size());
            const auto inputString = std::string{input.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &pullCallback.get());

    auto pullSocket =
        context_.PullSocket(pullCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &pullSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pull, pullSocket->Type());

    pullSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    pullSocket->Start(endpoint_);

    auto pushSocket =
        context_.PushSocket(zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &pushSocket.get());
    ASSERT_EQ(zmq::socket::Type::Push, pushSocket->Type());

    pushSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    pushSocket->Start(endpoint_);

    auto sent = pushSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) {
        ot::Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(callbackFinished);
}
}  // namespace ottest

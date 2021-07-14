// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <chrono>
#include <ctime>
#include <string>

#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

using namespace opentxs;

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
        : context_(Context().ZMQ())
    {
    }
};

TEST_F(Test_PushPull, Push_Pull)
{
    bool callbackFinished{false};

    auto pullCallback = zmq::ListenCallback::Factory(
        [this, &callbackFinished](zmq::Message& input) -> void {
            EXPECT_EQ(1, input.size());
            const std::string& inputString = *input.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &pullCallback.get());

    auto pullSocket =
        context_.PullSocket(pullCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &pullSocket.get());
    ASSERT_EQ(SocketType::Pull, pullSocket->Type());

    pullSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    pullSocket->Start(endpoint_);

    auto pushSocket =
        context_.PushSocket(zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &pushSocket.get());
    ASSERT_EQ(SocketType::Push, pushSocket->Type());

    pushSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    pushSocket->Start(endpoint_);

    auto sent = pushSocket->Send(testMessage_);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(callbackFinished);
}
}  // namespace ottest

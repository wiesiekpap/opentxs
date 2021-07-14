// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <future>
#include <string>

#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_PushSubscribe : public ::testing::Test
{
public:
    const zmq::Context& context_;
    const std::string testMessage_;
    const std::string endpoint_1_;
    const std::string endpoint_2_;
    std::atomic<int> counter_1_;
    std::atomic<int> counter_2_;
    std::atomic<int> counter_3_;

    Test_PushSubscribe()
        : context_(ot::Context().ZMQ())
        , testMessage_("zeromq test message")
        , endpoint_1_("inproc://opentxs/test/push_subscribe_test")
        , endpoint_2_("inproc://opentxs/test/publish_subscribe_test")
        , counter_1_(0)
        , counter_2_(0)
        , counter_3_(0)
    {
    }
};

TEST_F(Test_PushSubscribe, Push_Subscribe)
{
    auto promise = std::promise<bool>{};
    auto future = promise.get_future();
    auto callback = zmq::ListenCallback::Factory([&promise, this](auto& input) {
        const auto body = input.Body();

        EXPECT_GT(body.size(), 0);

        if (0 < body.size()) {
            const auto compare = testMessage_.compare(body.at(0));

            EXPECT_EQ(compare, 0);

            promise.set_value(0 == compare);
        } else {
            promise.set_value(false);
        }
    });

    auto sender = context_.PushSocket(zmq::socket::Socket::Direction::Bind);
    auto receiver = context_.SubscribeSocket(callback);
    auto message = context_.Message();
    message->AddFrame();
    message->AddFrame(testMessage_);

    ASSERT_TRUE(sender->Start(endpoint_1_));
    ASSERT_TRUE(receiver->Start(endpoint_1_));
    ASSERT_TRUE(sender->Send(message));

    const auto result = future.wait_for(std::chrono::seconds(10));

    ASSERT_EQ(result, std::future_status::ready);
    EXPECT_TRUE(future.get());
}

TEST_F(Test_PushSubscribe, Push_Publish_Subscribe)
{
    const std::string message1{"1"};
    const std::string message2{"2"};
    auto promise1 = std::promise<void>{};
    auto promise2 = std::promise<void>{};
    auto promise3 = std::promise<void>{};
    auto promise4 = std::promise<void>{};
    auto future1 = promise1.get_future();
    auto future2 = promise2.get_future();
    auto future3 = promise3.get_future();
    auto future4 = promise4.get_future();
    auto callback1 = zmq::ListenCallback::Factory([&](auto& input) {
        ++counter_1_;

        if (0 == message1.compare(input.Body_at(0))) {
            promise1.set_value();
        } else {
            promise4.set_value();
        }
    });
    auto callback2 = zmq::ListenCallback::Factory([&](auto& input) {
        ++counter_2_;

        if (0 == message1.compare(input.Body_at(0))) {
            promise2.set_value();
        } else {
            promise4.set_value();
        }
    });
    auto callback3 = zmq::ListenCallback::Factory([&](auto& input) {
        ++counter_3_;

        if (0 == message1.compare(input.Body_at(0))) {
            promise3.set_value();
        } else {
            promise4.set_value();
        }
    });
    auto sender1 = context_.PublishSocket();
    auto sender2 = context_.PushSocket(zmq::socket::Socket::Direction::Bind);
    auto receiver1 = context_.SubscribeSocket(callback1);
    auto receiver2 = context_.SubscribeSocket(callback2);
    auto receiver3 = context_.SubscribeSocket(callback3);

    ASSERT_TRUE(sender1->Start(endpoint_2_));
    ASSERT_TRUE(sender2->Start(endpoint_1_));
    ASSERT_TRUE(receiver1->Start(endpoint_1_));
    ASSERT_TRUE(receiver1->Start(endpoint_2_));
    ASSERT_TRUE(receiver2->Start(endpoint_1_));
    ASSERT_TRUE(receiver2->Start(endpoint_2_));
    ASSERT_TRUE(receiver3->Start(endpoint_1_));
    ASSERT_TRUE(receiver3->Start(endpoint_2_));

    ASSERT_EQ(counter_1_.load(), 0);
    ASSERT_EQ(counter_2_.load(), 0);
    ASSERT_EQ(counter_3_.load(), 0);

    // All receivers should get the message
    ASSERT_TRUE(sender1->Send(message1));

    const auto result1 = future1.wait_for(std::chrono::seconds(10));
    const auto result2 = future2.wait_for(std::chrono::seconds(2));
    const auto result3 = future3.wait_for(std::chrono::seconds(2));

    ASSERT_EQ(result1, std::future_status::ready);
    ASSERT_EQ(result2, std::future_status::ready);
    ASSERT_EQ(result3, std::future_status::ready);
    ASSERT_EQ(1, counter_1_.load());
    ASSERT_EQ(1, counter_2_.load());
    ASSERT_EQ(1, counter_3_.load());

    // One receivers should get the message
    ASSERT_TRUE(sender2->Send(message2));

    const auto result4 = future4.wait_for(std::chrono::seconds(10));

    ASSERT_EQ(result4, std::future_status::ready);
    ASSERT_EQ(4, counter_1_.load() + counter_2_.load() + counter_3_.load());
}
}  // namespace ottest

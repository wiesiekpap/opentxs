// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <chrono>
#include <ctime>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

using namespace opentxs;

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_RequestReply : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};

    const std::string endpoint_{"inproc://opentxs/test/request_reply_test"};
    const std::string endpoint2_{"inproc://opentxs/test/request_reply_test2"};

    void requestSocketThread(const std::string& msg);
    void replySocketThread(const std::string& endpoint);

    Test_RequestReply()
        : context_(Context().ZMQ())
    {
    }
};

void Test_RequestReply::requestSocketThread(const std::string& msg)
{
    auto requestSocket = context_.RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint_);

    auto [result, message] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_EQ(result, SendResult::VALID_REPLY);

    const auto messageString = std::string{message.Body().begin()->Bytes()};
    ASSERT_EQ(msg, messageString);
}

void Test_RequestReply::replySocketThread(const std::string& endpoint)
{
    bool replyReturned{false};

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this,
         &replyReturned](zmq::Message&& input) -> network::zeromq::Message {
            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);
            replyReturned = true;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.ReplySocket(
        replyCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint);

    auto end = std::time(nullptr) + 15;
    while (!replyReturned && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyReturned);
}

TEST_F(Test_RequestReply, Request_Reply)
{
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](zmq::Message&& input) -> network::zeromq::Message {
            const auto inputString = std::string{input.Body().begin()->Bytes()};
            EXPECT_EQ(testMessage_, inputString);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.ReplySocket(
        replyCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    auto requestSocket = context_.RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint_);

    auto [result, message] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_EQ(result, SendResult::VALID_REPLY);

    const auto messageString = std::string{message.Body().begin()->Bytes()};
    ASSERT_EQ(testMessage_, messageString);
}

TEST_F(Test_RequestReply, Request_2_Reply_1)
{
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](zmq::Message&& input) -> network::zeromq::Message {
            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.ReplySocket(
        replyCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    std::thread requestSocketThread1(
        &Test_RequestReply::requestSocketThread, this, testMessage2_);
    std::thread requestSocketThread2(
        &Test_RequestReply::requestSocketThread, this, testMessage3_);

    requestSocketThread1.join();
    requestSocketThread2.join();
}

TEST_F(Test_RequestReply, Request_1_Reply_2)
{
    std::thread replySocketThread1(
        &Test_RequestReply::replySocketThread, this, endpoint_);
    std::thread replySocketThread2(
        &Test_RequestReply::replySocketThread, this, endpoint2_);

    auto requestSocket = context_.RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint_);
    requestSocket->Start(endpoint2_);

    auto [result, message] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage2_);

        return out;
    }());

    ASSERT_EQ(result, SendResult::VALID_REPLY);

    auto messageString = std::string{message.Body().begin()->Bytes()};
    ASSERT_EQ(testMessage2_, messageString);

    auto [result2, message2] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage3_);

        return out;
    }());

    ASSERT_EQ(result2, SendResult::VALID_REPLY);

    messageString = message2.Body().begin()->Bytes();
    ASSERT_EQ(testMessage3_, messageString);

    replySocketThread1.join();
    replySocketThread2.join();
}

TEST_F(Test_RequestReply, Request_Reply_Multipart)
{
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const auto& input) -> network::zeromq::Message {
            EXPECT_EQ(4, input.size());
            EXPECT_EQ(1, input.Header().size());
            EXPECT_EQ(2, input.Body().size());

            for (const auto& frame : input.Header()) {
                EXPECT_EQ(testMessage_, std::string{frame.Bytes()});
            }

            for (const auto& frame : input.Body()) {
                const auto str = std::string{frame.Bytes()};
                bool match = (str == testMessage2_) || (str == testMessage3_);

                EXPECT_TRUE(match);
            }

            auto reply = ot::network::zeromq::reply_to_message(input);
            for (const auto& frame : input.Body()) { reply.AddFrame(frame); }
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.ReplySocket(
        replyCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    auto requestSocket = context_.RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint_);

    auto multipartMessage = opentxs::network::zeromq::Message{};
    multipartMessage.AddFrame(testMessage_);
    multipartMessage.StartBody();
    multipartMessage.AddFrame(testMessage2_);
    multipartMessage.AddFrame(testMessage3_);

    auto [result, message] = requestSocket->Send(std::move(multipartMessage));

    ASSERT_EQ(result, SendResult::VALID_REPLY);

    const auto messageHeader = std::string{message.Header().begin()->Bytes()};

    ASSERT_EQ(testMessage_, messageHeader);

    for (const auto& frame : message.Body()) {
        bool match = (frame.Bytes() == testMessage2_) ||
                     (frame.Bytes() == testMessage3_);
        ASSERT_TRUE(match);
    }
}
}  // namespace ottest

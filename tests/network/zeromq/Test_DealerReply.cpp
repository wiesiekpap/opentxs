// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <chrono>
#include <ctime>
#include <string>
#include <thread>

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
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

using namespace opentxs;

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_DealerReply : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};

    const std::string endpoint_{"inproc://opentxs/test/dealer_reply_test"};

    void dealerSocketThread(const std::string& msg);

    Test_DealerReply()
        : context_(Context().ZMQ())
    {
    }
};

void Test_DealerReply::dealerSocketThread(const std::string& msg)
{
    bool replyProcessed{false};
    auto listenCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message& input) -> void {
            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto dealerSocket = context_.DealerSocket(
        listenCallback, zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint_);

    auto message = context_.Message(msg);
    message->PrependEmptyFrame();
    auto sent = dealerSocket->Send(message);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}

TEST_F(Test_DealerReply, Dealer_Reply)
{
    bool replyReturned{false};
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this, &replyReturned](const zmq::Message& input) -> OTZMQMessage {
            EXPECT_EQ(1, input.size());
            EXPECT_EQ(input.Header().size(), 0);
            EXPECT_EQ(1, input.Body().size());

            const std::string inputString{*input.Body().begin()};

            EXPECT_EQ(testMessage_, inputString);

            auto reply = context_.ReplyMessage(input);

            EXPECT_EQ(reply->size(), 0);
            EXPECT_EQ(reply->Header().size(), 0);
            EXPECT_EQ(reply->Body().size(), 0);

            reply->AddFrame(inputString);

            EXPECT_EQ(1, reply->size());
            EXPECT_EQ(reply->Header().size(), 0);
            EXPECT_EQ(1, reply->Body().size());
            EXPECT_EQ(inputString, std::string(reply->Body_at(0)));

            replyReturned = true;

            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.ReplySocket(
        replyCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message& input) -> void {
            EXPECT_EQ(2, input.size());
            EXPECT_EQ(input.Header().size(), 0);
            EXPECT_EQ(1, input.Body().size());

            const std::string& inputString = *input.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.DealerSocket(
        dealerCallback, zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint_);

    auto message = context_.Message(testMessage_);

    ASSERT_TRUE(1 == message->size());
    ASSERT_TRUE(0 == message->Header().size());
    ASSERT_TRUE(1 == message->Body().size());
    ASSERT_EQ(testMessage_, std::string(message->Body_at(0)));
    ASSERT_EQ(testMessage_, std::string(message->at(0)));

    message->PrependEmptyFrame();

    ASSERT_TRUE(2 == message->size());
    ASSERT_TRUE(0 == message->Header().size());
    ASSERT_TRUE(1 == message->Body().size());

    auto sent = dealerSocket->Send(message);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyReturned && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyReturned);

    end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}

TEST_F(Test_DealerReply, Dealer_2_Reply_1)
{
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const zmq::Message& input) -> OTZMQMessage {
            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            auto reply = context_.ReplyMessage(input);
            reply->AddFrame(inputString);
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.ReplySocket(
        replyCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    std::thread dealerSocketThread1(
        &Test_DealerReply::dealerSocketThread, this, testMessage2_);
    std::thread dealerSocketThread2(
        &Test_DealerReply::dealerSocketThread, this, testMessage3_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
}

TEST_F(Test_DealerReply, Dealer_Reply_Multipart)
{
    bool replyReturned{false};
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this, &replyReturned](const zmq::Message& input) -> OTZMQMessage {
            // ReplySocket removes the delimiter frame.
            EXPECT_EQ(4, input.size());
            EXPECT_EQ(1, input.Header().size());
            EXPECT_EQ(2, input.Body().size());

            for (const std::string frame : input.Header()) {
                EXPECT_EQ(testMessage_, frame);
            }

            for (const std::string frame : input.Body()) {
                bool match = frame == testMessage2_ || frame == testMessage3_;

                EXPECT_TRUE(match);
            }

            auto reply = context_.ReplyMessage(input);
            for (const auto& frame : input.Body()) { reply->AddFrame(frame); }
            replyReturned = true;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.ReplySocket(
        replyCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message& input) -> void {
            // ReplySocket puts the delimiter frame back when it sends the
            // reply.
            ASSERT_EQ(5, input.size());
            ASSERT_EQ(input.Header().size(), 0);
            ASSERT_EQ(4, input.Body().size());

            const std::string& header = input.at(1);

            ASSERT_EQ(testMessage_, header);

            for (auto i{3u}; i < input.size(); ++i) {
                const std::string& frame = input.at(i);
                bool match = frame == testMessage2_ || frame == testMessage3_;

                EXPECT_TRUE(match);
            }

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.DealerSocket(
        dealerCallback, zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint_);

    auto multipartMessage = context_.Message(testMessage_);
    multipartMessage->AddFrame();
    multipartMessage->AddFrame(testMessage2_);
    multipartMessage->AddFrame(testMessage3_);
    // Prepend a delimiter frame for the ReplySocket.
    multipartMessage->PrependEmptyFrame();

    auto sent = dealerSocket->Send(multipartMessage);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyReturned && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyReturned);

    end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}
}  // namespace ottest

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <ctime>
#include <map>
#include <string>
#include <thread>
#include <utility>

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
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

using namespace opentxs;

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_DealerRouter : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};

    const std::string endpoint_{"inproc://opentxs/test/dealer_router_test"};
    const std::string endpoint2_{"inproc://opentxs/test/dealer_router_test2"};

    std::atomic_int callbackFinishedCount_{0};

    int callbackCount_{0};

    void dealerSocketThread(const std::string& msg);
    void routerSocketThread(const std::string& endpoint);

    Test_DealerRouter()
        : context_(Context().ZMQ())
    {
    }
};

void Test_DealerRouter::dealerSocketThread(const std::string& msg)
{
    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message& input) -> void {
            EXPECT_EQ(2, input.size());

            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

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

    auto sent = dealerSocket->Send(msg);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}

void Test_DealerRouter::routerSocketThread(const std::string& endpoint)
{
    auto replyMessage = context_.Message();

    bool replyProcessed{false};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage, &replyProcessed](zmq::Message& input) -> void {
            EXPECT_EQ(3, input.size());

            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            replyMessage = context_.ReplyMessage(input);
            for (const auto& frame : input.Body()) {
                replyMessage->AddFrame(frame);
            }

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.RouterSocket(
        routerCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(SocketType::Router, routerSocket->Type());

    routerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    routerSocket->Start(endpoint);

    auto end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(replyProcessed);

    routerSocket->Send(replyMessage);

    // Give the router socket time to send the message.
    Sleep(std::chrono::milliseconds(500));
}

TEST_F(Test_DealerRouter, Dealer_Router)
{
    auto replyMessage = context_.Message();

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage](zmq::Message& input) -> void {
            EXPECT_EQ(3, input.size());
            const std::string& inputString = *input.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            replyMessage = context_.ReplyMessage(input);
            for (auto& frame : input.Body()) { replyMessage->AddFrame(frame); }
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.RouterSocket(
        routerCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(SocketType::Router, routerSocket->Type());

    routerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    routerSocket->Start(endpoint_);

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message& input) -> void {
            EXPECT_EQ(2, input.size());
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

    auto sent = dealerSocket->Send(testMessage_);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (0 == replyMessage->size() && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_NE(0, replyMessage->size());

    sent = routerSocket->Send(replyMessage);

    ASSERT_TRUE(sent);

    end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}

TEST_F(Test_DealerRouter, Dealer_2_Router_1)
{
    callbackCount_ = 2;

    std::map<std::string, OTZMQMessage> replyMessages{
        std::pair<std::string, OTZMQMessage>(testMessage2_, context_.Message()),
        std::pair<std::string, OTZMQMessage>(
            testMessage3_, context_.Message())};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessages](zmq::Message& input) -> void {
            EXPECT_EQ(3, input.size());

            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            auto& replyMessage = replyMessages.at(inputString);
            replyMessage = context_.ReplyMessage(input);
            for (const std::string frame : input.Body()) {
                replyMessage->AddFrame(frame);
            }

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.RouterSocket(
        routerCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(SocketType::Router, routerSocket->Type());

    routerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    routerSocket->Start(endpoint_);

    std::thread dealerSocketThread1(
        &Test_DealerRouter::dealerSocketThread, this, testMessage2_);
    std::thread dealerSocketThread2(
        &Test_DealerRouter::dealerSocketThread, this, testMessage3_);

    auto& replyMessage1 = replyMessages.at(testMessage2_);
    auto& replyMessage2 = replyMessages.at(testMessage3_);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinishedCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool message1Sent{false};
    if (0 != replyMessage1->size()) {
        routerSocket->Send(replyMessage1);
        message1Sent = true;
    } else {
        routerSocket->Send(replyMessage2);
    }

    end = std::time(nullptr) + 15;
    while (callbackFinishedCount_ < callbackCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (false == message1Sent) {
        routerSocket->Send(replyMessage1);
    } else {
        routerSocket->Send(replyMessage2);
    }

    ASSERT_EQ(callbackCount_, callbackFinishedCount_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
}

TEST_F(Test_DealerRouter, Dealer_1_Router_2)
{
    callbackCount_ = 2;

    std::thread routerSocketThread1(
        &Test_DealerRouter::routerSocketThread, this, endpoint_);
    std::thread routerSocketThread2(
        &Test_DealerRouter::routerSocketThread, this, endpoint2_);

    auto dealerCallback =
        zmq::ListenCallback::Factory([this](const zmq::Message& input) -> void {
            EXPECT_EQ(2, input.size());
            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            ++callbackFinishedCount_;
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
    dealerSocket->Start(endpoint2_);

    auto sent = dealerSocket->Send(testMessage2_);

    ASSERT_TRUE(sent);

    sent = dealerSocket->Send(testMessage3_);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (callbackFinishedCount_ < callbackCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(callbackCount_, callbackFinishedCount_);

    routerSocketThread1.join();
    routerSocketThread2.join();
}

TEST_F(Test_DealerRouter, Dealer_Router_Multipart)
{
    auto replyMessage = context_.Message();

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage](zmq::Message& input) -> void {
            EXPECT_EQ(5, input.size());
            // Original header + identity frame.
            EXPECT_EQ(2, input.Header().size());
            EXPECT_EQ(2, input.Body().size());

            auto originalFound{false};
            for (const std::string frame : input.Header()) {
                if (testMessage_ == frame) { originalFound = true; }
            }

            EXPECT_TRUE(originalFound);

            for (const std::string frame : input.Body()) {
                bool match = frame == testMessage2_ || frame == testMessage3_;
                EXPECT_TRUE(match);
            }

            replyMessage = context_.ReplyMessage(input);
            for (auto& frame : input.Body()) { replyMessage->AddFrame(frame); }
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.RouterSocket(
        routerCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(SocketType::Router, routerSocket->Type());

    routerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    routerSocket->Start(endpoint_);

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message& input) -> void {
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

    auto sent = dealerSocket->Send(multipartMessage);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (0 == replyMessage->size() && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_NE(0, replyMessage->size());

    sent = routerSocket->Send(replyMessage);

    ASSERT_TRUE(sent);

    end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}
}  // namespace ottest

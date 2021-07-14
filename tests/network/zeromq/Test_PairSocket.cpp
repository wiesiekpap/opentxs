// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <chrono>
#include <ctime>
#include <future>
#include <string>
#include <thread>

#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"

using namespace opentxs;

#define TEST_ENDPOINT "inproc://opentxs/pairsocket_endpoint"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_PairSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};

    OTZMQPairSocket* pairSocket_;

    void pairSocketThread(const std::string& msg, std::promise<void>* promise);

    Test_PairSocket()
        : context_(Context().ZMQ())
        , pairSocket_(nullptr)
    {
    }
    Test_PairSocket(const Test_PairSocket&) = delete;
    Test_PairSocket(Test_PairSocket&&) = delete;
    Test_PairSocket& operator=(const Test_PairSocket&) = delete;
    Test_PairSocket& operator=(Test_PairSocket&&) = delete;
};

void Test_PairSocket::pairSocketThread(
    const std::string& message,
    std::promise<void>* promise)
{
    struct Cleanup {
        Cleanup(std::promise<void>& promise)
            : promise_(promise)
        {
        }

        ~Cleanup()
        {
            try {
                promise_.set_value();
            } catch (...) {
            }
        }

    private:
        std::promise<void>& promise_;
    };

    auto cleanup = Cleanup(*promise);
    bool callbackFinished = false;
    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [&callbackFinished, &message](network::zeromq::Message& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const std::string& inputString = *msg.Body().begin();

            EXPECT_EQ(message, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());
    ASSERT_NE(nullptr, pairSocket_);

    auto pairSocket = context_.PairSocket(listenCallback, *pairSocket_);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(SocketType::Pair, pairSocket->Type());

    promise->set_value();
    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(Test_PairSocket, PairSocket_Factory1)
{
    auto pairSocket =
        context_.PairSocket(network::zeromq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(SocketType::Pair, pairSocket->Type());
}

TEST_F(Test_PairSocket, PairSocket_Factory2)
{
    auto peer = context_.PairSocket(network::zeromq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(SocketType::Pair, peer->Type());

    auto pairSocket =
        context_.PairSocket(network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(SocketType::Pair, pairSocket->Type());
    ASSERT_EQ(pairSocket->Endpoint(), peer->Endpoint());
}

TEST_F(Test_PairSocket, PairSocket_Factory3)
{
    auto pairSocket = context_.PairSocket(
        network::zeromq::ListenCallback::Factory(), TEST_ENDPOINT);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(SocketType::Pair, pairSocket->Type());
    ASSERT_EQ(pairSocket->Endpoint(), TEST_ENDPOINT);
}

TEST_F(Test_PairSocket, PairSocket_Send1)
{
    bool callbackFinished = false;

    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](network::zeromq::Message& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const std::string& inputString = *msg.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto peer = context_.PairSocket(listenCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(SocketType::Pair, peer->Type());

    auto pairSocket =
        context_.PairSocket(network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(SocketType::Pair, pairSocket->Type());

    auto sent = pairSocket->Send(testMessage_);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(Test_PairSocket, PairSocket_Send2)
{
    bool callbackFinished = false;

    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](network::zeromq::Message& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const std::string& inputString = *msg.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto peer = context_.PairSocket(listenCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(SocketType::Pair, peer->Type());

    auto pairSocket =
        context_.PairSocket(network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(SocketType::Pair, pairSocket->Type());

    auto data = Data::Factory(testMessage_.data(), testMessage_.size());

    ASSERT_NE(nullptr, &data.get());

    auto sent = pairSocket->Send(data);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(Test_PairSocket, PairSocket_Send3)
{
    bool callbackFinished = false;

    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](network::zeromq::Message& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const std::string& inputString = *msg.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto peer = context_.PairSocket(listenCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(SocketType::Pair, peer->Type());

    auto pairSocket =
        context_.PairSocket(network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(SocketType::Pair, pairSocket->Type());

    auto msg = context_.Message(testMessage_);

    ASSERT_NE(nullptr, &msg.get());

    auto sent = pairSocket->Send(msg);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(Test_PairSocket, PairSocket_Send_Two_Way)
{
    bool peerCallbackFinished = false;

    auto peerCallback = network::zeromq::ListenCallback::Factory(
        [this, &peerCallbackFinished](network::zeromq::Message& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const std::string& inputString = *msg.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            peerCallbackFinished = true;
        });

    ASSERT_NE(nullptr, &peerCallback.get());

    auto peer = context_.PairSocket(peerCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(SocketType::Pair, peer->Type());

    bool callbackFinished = false;

    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](network::zeromq::Message& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const std::string& inputString = *msg.Body().begin();

            EXPECT_EQ(testMessage2_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto pairSocket = context_.PairSocket(listenCallback, peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(SocketType::Pair, pairSocket->Type());

    auto sent = pairSocket->Send(testMessage_);

    ASSERT_TRUE(sent);

    sent = peer->Send(testMessage2_);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!peerCallbackFinished && !callbackFinished &&
           std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(peerCallbackFinished);
    ASSERT_TRUE(callbackFinished);
}

TEST_F(Test_PairSocket, PairSocket_Send_Separate_Thread)
{
    auto pairSocket =
        context_.PairSocket(network::zeromq::ListenCallback::Factory());
    pairSocket_ = &pairSocket;
    auto promise = std::promise<void>{};
    auto future = promise.get_future();

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(SocketType::Pair, pairSocket->Type());

    std::thread pairSocketThread1(
        &Test_PairSocket::pairSocketThread, this, testMessage_, &promise);
    future.get();
    auto sent = pairSocket->Send(testMessage_);

    ASSERT_TRUE(sent);

    pairSocketThread1.join();
}
}  // namespace ottest

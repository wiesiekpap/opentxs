// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"

using namespace opentxs;

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_RouterSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    Test_RouterSocket()
        : context_(Context().ZMQ())
    {
    }
};

TEST_F(Test_RouterSocket, RouterSocket_Factory)
{
    auto dealerSocket = context_.RouterSocket(
        zmq::ListenCallback::Factory(),
        zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, dealerSocket->Type());
}
}  // namespace ottest

// TODO: Add tests for other public member functions: SetPublicKey,
// SetSocksProxy

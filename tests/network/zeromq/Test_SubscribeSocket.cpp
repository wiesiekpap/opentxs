// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <string>

#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

using namespace opentxs;

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_SubscribeSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const std::string testMessage_{"zeromq test message"};

    Test_SubscribeSocket()
        : context_(Context().ZMQ())
    {
    }
};

TEST_F(Test_SubscribeSocket, SubscribeSocket_Factory)
{
    auto subscribeSocket =
        context_.SubscribeSocket(network::zeromq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(SocketType::Subscribe, subscribeSocket->Type());
}
}  // namespace ottest

// TODO: Add tests for other public member functions: SetPublicKey,
// SetSocksProxy

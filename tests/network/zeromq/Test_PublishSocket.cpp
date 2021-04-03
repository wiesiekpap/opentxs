// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"

using namespace opentxs;

namespace zmq = ot::network::zeromq;

namespace
{
class Test_PublishSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    Test_PublishSocket()
        : context_(Context().ZMQ())
    {
    }
};
}  // namespace

TEST_F(Test_PublishSocket, PublishSocket_Factory)
{
    auto publishSocket = context_.PublishSocket();

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(SocketType::Publish, publishSocket->Type());
}

// TODO: Add tests for other public member functions: SetPrivateKey

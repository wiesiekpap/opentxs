// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <string>

#include "2_Factory.hpp"
#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/Pimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

using namespace opentxs;

namespace
{
class Test_ListenCallback : public ::testing::Test
{
public:
    const std::string testMessage_{"zeromq test message"};
};
}  // namespace

TEST(ListenCallback, ListenCallback_Factory)
{
    auto listenCallback =
        network::zeromq::ListenCallback::Factory([](auto&) -> void {});

    ASSERT_NE(nullptr, &listenCallback.get());
}

TEST_F(Test_ListenCallback, ListenCallback_Process)
{
    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this](network::zeromq::Message& input) -> void {
            const std::string& inputString = *input.Body().begin();
            EXPECT_EQ(testMessage_, inputString);
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto testMessage = OTZMQMessage{
        Factory::ZMQMessage(testMessage_.data(), testMessage_.size())};

    ASSERT_NE(nullptr, &testMessage.get());

    listenCallback->Process(testMessage);
}

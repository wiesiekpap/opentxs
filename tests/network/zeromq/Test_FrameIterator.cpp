// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <string>

#include "opentxs/Version.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"

using namespace opentxs;

TEST(FrameIterator, constructors)
{
    auto multipartMessage = network::zeromq::Message{};

    auto frameIterator{multipartMessage.begin()};
    ASSERT_EQ(multipartMessage.begin(), frameIterator);

    multipartMessage.AddFrame(std::string{"msg1"});
    multipartMessage.AddFrame(std::string{"msg2"});

    auto frameIterator3(++frameIterator);
    auto& message = *frameIterator3;
    auto messageString = std::string{message.Bytes()};
    ASSERT_STREQ("msg2", messageString.c_str());
}

TEST(FrameIterator, assignment_operator)
{
    auto multipartMessage = network::zeromq::Message{};

    auto frameIterator{multipartMessage.begin()};
    ASSERT_EQ(multipartMessage.begin(), frameIterator);

    multipartMessage.AddFrame(std::string{"msg1"});
    multipartMessage.AddFrame(std::string{"msg2"});

    auto frameIterator2{++frameIterator};
    auto& frameIterator3 = frameIterator2;
    auto& message = *frameIterator3;
    auto messageString = std::string{message.Bytes()};
    ASSERT_STREQ("msg2", messageString.c_str());
}

TEST(FrameIterator, operator_asterisk)
{
    auto multipartMessage = network::zeromq::Message{};

    multipartMessage.AddFrame(std::string{"msg1"});

    auto frameIterator{multipartMessage.begin()};
    auto& message = *frameIterator;
    auto messageString = std::string{message.Bytes()};
    ASSERT_STREQ("msg1", messageString.c_str());
}

TEST(FrameIterator, operator_asterisk_const)
{
    auto multipartMessage = network::zeromq::Message{};

    multipartMessage.AddFrame(std::string{"msg1"});

    const auto frameIterator = multipartMessage.begin();
    auto& message = *frameIterator;
    auto messageString = std::string{message.Bytes()};
    ASSERT_STREQ("msg1", messageString.c_str());
}

TEST(FrameIterator, operator_equal)
{
    auto message = network::zeromq::Message{};

    EXPECT_TRUE(message.begin() == message.end());

    message.AddFrame(std::string{"msg1"});

    EXPECT_FALSE(message.begin() == message.end());

    auto frameIterator2{message.begin()};
    auto& frameIterator3 = frameIterator2;

    EXPECT_TRUE(frameIterator2 == frameIterator3);
    EXPECT_FALSE(message.end() == frameIterator2);
}

TEST(FrameIterator, operator_notEqual)
{
    auto message = network::zeromq::Message{};

    EXPECT_FALSE(message.begin() != message.end());

    message.AddFrame(std::string{"msg1"});

    EXPECT_TRUE(message.begin() != message.end());

    auto frameIterator2{message.begin()};
    auto& frameIterator3 = frameIterator2;

    EXPECT_FALSE(frameIterator2 != frameIterator3);
    EXPECT_TRUE(message.begin() == frameIterator2);
}

TEST(FrameIterator, operator_pre_increment)
{
    auto multipartMessage = network::zeromq::Message{};

    multipartMessage.AddFrame(std::string{"msg1"});
    multipartMessage.AddFrame(std::string{"msg2"});

    auto frameIterator{multipartMessage.begin()};
    auto& frameIterator2 = ++frameIterator;

    auto& message = *frameIterator2;
    auto stringMessage = std::string{message.Bytes()};
    ASSERT_STREQ("msg2", stringMessage.c_str());

    auto& message2 = *frameIterator;
    stringMessage = message2.Bytes();
    ASSERT_STREQ("msg2", stringMessage.c_str());
}

TEST(FrameIterator, operator_post_increment)
{
    auto multipartMessage = network::zeromq::Message{};

    multipartMessage.AddFrame(std::string{"msg1"});
    multipartMessage.AddFrame(std::string{"msg2"});

    auto frameIterator{multipartMessage.begin()};
    auto frameIterator2 = frameIterator++;

    auto& message = *frameIterator2;
    auto stringMessage = std::string{message.Bytes()};
    ASSERT_STREQ("msg1", stringMessage.c_str());

    auto& message2 = *frameIterator;
    stringMessage = message2.Bytes();
    ASSERT_STREQ("msg2", stringMessage.c_str());
}

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Message.hpp"

using namespace opentxs;

TEST(FrameIterator, constructors)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    network::zeromq::FrameIterator frameIterator(multipartMessage->begin());
    ASSERT_EQ(multipartMessage->begin(), frameIterator);

    network::zeromq::FrameIterator frameIterator2(&multipartMessage.get());
    ASSERT_EQ(multipartMessage->begin(), frameIterator2);

    multipartMessage->AddFrame(std::string{"msg1"});
    multipartMessage->AddFrame(std::string{"msg2"});

    network::zeromq::FrameIterator frameIterator3(&multipartMessage.get(), 1);
    auto& message = *frameIterator3;
    std::string messageString = message;
    ASSERT_STREQ("msg2", messageString.c_str());
}

TEST(FrameIterator, assignment_operator)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    network::zeromq::FrameIterator frameIterator = multipartMessage->begin();
    ASSERT_EQ(multipartMessage->begin(), frameIterator);

    multipartMessage->AddFrame(std::string{"msg1"});
    multipartMessage->AddFrame(std::string{"msg2"});

    network::zeromq::FrameIterator frameIterator2(&multipartMessage.get(), 1);
    network::zeromq::FrameIterator& frameIterator3 = frameIterator2;
    auto& message = *frameIterator3;
    std::string messageString = message;
    ASSERT_STREQ("msg2", messageString.c_str());
}

TEST(FrameIterator, operator_asterisk)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    multipartMessage->AddFrame(std::string{"msg1"});

    network::zeromq::FrameIterator frameIterator = multipartMessage->begin();
    auto& message = *frameIterator;
    std::string messageString = message;
    ASSERT_STREQ("msg1", messageString.c_str());
}

TEST(FrameIterator, operator_asterisk_const)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    multipartMessage->AddFrame(std::string{"msg1"});

    const network::zeromq::FrameIterator frameIterator =
        multipartMessage->begin();
    auto& message = *frameIterator;
    std::string messageString = message;
    ASSERT_STREQ("msg1", messageString.c_str());
}

TEST(FrameIterator, operator_equal)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    ASSERT_TRUE(multipartMessage->begin() == multipartMessage->end());

    multipartMessage->AddFrame(std::string{"msg1"});

    ASSERT_FALSE(multipartMessage->begin() == multipartMessage->end());

    network::zeromq::FrameIterator frameIterator2(&multipartMessage.get(), 1);
    network::zeromq::FrameIterator& frameIterator3 = frameIterator2;

    ASSERT_TRUE(frameIterator2 == frameIterator3);
    ASSERT_FALSE(multipartMessage->begin() == frameIterator2);
}

TEST(FrameIterator, operator_notEqual)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    ASSERT_FALSE(multipartMessage->begin() != multipartMessage->end());

    multipartMessage->AddFrame(std::string{"msg1"});

    ASSERT_TRUE(multipartMessage->begin() != multipartMessage->end());

    network::zeromq::FrameIterator frameIterator2(&multipartMessage.get(), 1);
    network::zeromq::FrameIterator& frameIterator3 = frameIterator2;

    ASSERT_FALSE(frameIterator2 != frameIterator3);
    ASSERT_TRUE(multipartMessage->begin() != frameIterator2);
}

TEST(FrameIterator, operator_pre_increment)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    multipartMessage->AddFrame(std::string{"msg1"});
    multipartMessage->AddFrame(std::string{"msg2"});

    network::zeromq::FrameIterator frameIterator = multipartMessage->begin();
    network::zeromq::FrameIterator& frameIterator2 = ++frameIterator;

    auto& message = *frameIterator2;
    std::string stringMessage = message;
    ASSERT_STREQ("msg2", stringMessage.c_str());

    auto& message2 = *frameIterator;
    stringMessage = message2;
    ASSERT_STREQ("msg2", stringMessage.c_str());
}

TEST(FrameIterator, operator_post_increment)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    multipartMessage->AddFrame(std::string{"msg1"});
    multipartMessage->AddFrame(std::string{"msg2"});

    network::zeromq::FrameIterator frameIterator = multipartMessage->begin();
    network::zeromq::FrameIterator frameIterator2 = frameIterator++;

    auto& message = *frameIterator2;
    std::string stringMessage = message;
    ASSERT_STREQ("msg1", stringMessage.c_str());

    auto& message2 = *frameIterator;
    stringMessage = message2;
    ASSERT_STREQ("msg2", stringMessage.c_str());
}

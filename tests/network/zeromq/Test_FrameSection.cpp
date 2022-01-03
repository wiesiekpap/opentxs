// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cstddef>
#include <iterator>
#include <memory>
#include <string>

#include "internal/network/zeromq/message/FrameSection.hpp"
#include "network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"

using namespace opentxs;

TEST(FrameSection, constructors)
{
    auto multipartMessage = network::zeromq::Message{};

    network::zeromq::FrameSection frameSection(multipartMessage.Header());
    ASSERT_EQ(frameSection.size(), 0);

    auto frameSection2 = network::zeromq::FrameSection{
        std::make_unique<network::zeromq::implementation::FrameSection>(
            &multipartMessage, 0, 0)
            .release()};
    ASSERT_EQ(frameSection2.size(), 0);

    multipartMessage.AddFrame(std::string("msg1"));
    multipartMessage.AddFrame(std::string{"msg2"});

    network::zeromq::FrameSection frameSection3(multipartMessage.Body());
    ASSERT_EQ(frameSection3.size(), 2);

    auto frameSection4 = network::zeromq::FrameSection{
        std::make_unique<network::zeromq::implementation::FrameSection>(
            &multipartMessage, 0, 2)
            .release()};
    ASSERT_EQ(frameSection4.size(), 2);

    multipartMessage.AddFrame();
    multipartMessage.AddFrame(std::string{"msg3"});

    network::zeromq::FrameSection frameSection5(multipartMessage.Header());
    ASSERT_EQ(frameSection5.size(), 2);

    network::zeromq::FrameSection frameSection6(multipartMessage.Body());
    ASSERT_EQ(frameSection6.size(), 1);

    auto frameSection7 = network::zeromq::FrameSection{
        std::make_unique<network::zeromq::implementation::FrameSection>(
            &multipartMessage, 3, 1)
            .release()};
    ASSERT_EQ(frameSection7.size(), 1);
}

TEST(FrameSection, at)
{
    auto multipartMessage = network::zeromq::Message{};
    multipartMessage.AddFrame(std::string{"msg1"});
    multipartMessage.AddFrame(std::string{"msg2"});
    multipartMessage.AddFrame();

    auto header = multipartMessage.Header();

    const network::zeromq::Frame& header1 = header.at(0);
    auto msgString = std::string{header1.Bytes()};
    ASSERT_STREQ("msg1", msgString.c_str());

    const network::zeromq::Frame& header2 = header.at(1);
    msgString = header2.Bytes();
    ASSERT_STREQ("msg2", msgString.c_str());

    multipartMessage.AddFrame(std::string{"msg3"});
    multipartMessage.AddFrame(std::string{"msg4"});

    auto body = multipartMessage.Body();

    const network::zeromq::Frame& body1 = body.at(0);
    msgString = body1.Bytes();
    ASSERT_STREQ("msg3", msgString.c_str());

    const network::zeromq::Frame& body2 = body.at(1);
    msgString = body2.Bytes();
    ASSERT_STREQ("msg4", msgString.c_str());
}

TEST(FrameSection, begin)
{
    auto multipartMessage = network::zeromq::Message{};

    auto headerSection = multipartMessage.Header();
    network::zeromq::FrameIterator headerIterator = headerSection.begin();
    ASSERT_EQ(headerSection.end(), headerIterator);
    ASSERT_EQ(

        std::distance(headerIterator, headerSection.end()), 0);

    multipartMessage.AddFrame(std::string{"msg1"});
    multipartMessage.AddFrame(std::string{"msg2"});
    multipartMessage.AddFrame();

    auto headerSection2 = multipartMessage.Header();
    network::zeromq::FrameIterator headerIterator2 = headerSection.begin();
    ASSERT_NE(headerSection2.end(), headerIterator2);
    ASSERT_EQ(

        std::distance(headerIterator2, headerSection2.end()), 2);

    std::advance(headerIterator2, 2);
    ASSERT_EQ(headerSection2.end(), headerIterator2);
    ASSERT_EQ(

        std::distance(headerIterator2, headerSection2.end()), 0);

    auto bodySection = multipartMessage.Body();
    network::zeromq::FrameIterator bodyIterator = bodySection.begin();
    ASSERT_EQ(bodySection.end(), bodyIterator);
    ASSERT_EQ(

        std::distance(bodyIterator, bodySection.end()), 0);

    multipartMessage.AddFrame(std::string{"msg3"});
    multipartMessage.AddFrame(std::string{"msg4"});

    auto bodySection2 = multipartMessage.Header();
    network::zeromq::FrameIterator bodyIterator2 = headerSection.begin();
    ASSERT_NE(bodySection2.end(), bodyIterator2);
    ASSERT_EQ(

        std::distance(bodyIterator2, bodySection2.end()), 2);

    std::advance(bodyIterator2, 2);
    ASSERT_EQ(bodySection2.end(), bodyIterator2);
    ASSERT_EQ(

        std::distance(bodyIterator2, bodySection2.end()), 0);
}

TEST(FrameSection, end)
{
    auto multipartMessage = network::zeromq::Message{};

    auto headerSection = multipartMessage.Header();
    network::zeromq::FrameIterator headerIterator = headerSection.end();
    ASSERT_EQ(headerSection.begin(), headerIterator);
    ASSERT_EQ(

        std::distance(headerSection.begin(), headerIterator), 0);

    auto bodySection = multipartMessage.Body();
    network::zeromq::FrameIterator bodyIterator = bodySection.end();
    ASSERT_EQ(bodySection.begin(), bodyIterator);
    ASSERT_EQ(

        std::distance(bodySection.begin(), bodyIterator), 0);

    multipartMessage.AddFrame(std::string{"msg1"});
    multipartMessage.AddFrame(std::string{"msg2"});
    multipartMessage.AddFrame();

    auto headerSection2 = multipartMessage.Header();
    network::zeromq::FrameIterator headerIterator2 = headerSection2.end();
    ASSERT_NE(headerSection2.begin(), headerIterator2);
    ASSERT_EQ(

        std::distance(headerSection2.begin(), headerIterator2), 2);

    auto bodySection2 = multipartMessage.Body();
    network::zeromq::FrameIterator bodyIterator2 = bodySection2.end();
    ASSERT_EQ(bodySection2.begin(), bodyIterator2);
    ASSERT_EQ(

        std::distance(bodySection2.begin(), bodyIterator2), 0);

    multipartMessage.AddFrame(std::string{"msg3"});
    multipartMessage.AddFrame(std::string{"msg4"});

    auto bodySection3 = multipartMessage.Body();
    network::zeromq::FrameIterator bodyIterator3 = bodySection3.end();
    ASSERT_NE(bodySection3.begin(), bodyIterator3);
    ASSERT_EQ(

        std::distance(bodySection3.begin(), bodyIterator3), 2);
}

TEST(FrameSection, size)
{
    auto multipartMessage = network::zeromq::Message{};

    auto headerSection = multipartMessage.Header();
    std::size_t size = headerSection.size();
    ASSERT_EQ(size, 0);

    auto bodySection = multipartMessage.Body();
    size = bodySection.size();
    ASSERT_EQ(size, 0);

    multipartMessage.AddFrame(std::string{"msg1"});
    multipartMessage.AddFrame(std::string{"msg2"});
    multipartMessage.AddFrame();

    auto headerSection2 = multipartMessage.Header();
    size = headerSection2.size();
    ASSERT_EQ(size, 2);

    auto bodySection2 = multipartMessage.Body();
    size = bodySection2.size();
    ASSERT_EQ(size, 0);

    multipartMessage.AddFrame(std::string{"msg3"});
    multipartMessage.AddFrame(std::string{"msg4"});

    auto bodySection3 = multipartMessage.Body();
    size = bodySection3.size();
    ASSERT_EQ(size, 2);
}

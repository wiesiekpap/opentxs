// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <future>
#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace ot = opentxs;
namespace zmq = opentxs::network::zeromq;

namespace ottest
{
class ListenCallback : public ::testing::Test
{
private:
    std::promise<std::string> promise_;

protected:
    const std::string testMessage_;
    std::future<std::string> future_;
    ot::OTZMQListenCallback callback_;

    ListenCallback() noexcept
        : promise_()
        , testMessage_("zeromq test message")
        , future_(promise_.get_future())
        , callback_(zmq::ListenCallback::Factory([&](auto& input) -> void {
            promise_.set_value(std::string{input.at(0).Bytes()});
        }))
    {
    }
};

TEST_F(ListenCallback, ListenCallback_Process)
{
    auto message = [&] {
        auto out = zmq::Message::Factory();
        out->AddFrame(testMessage_);

        return out;
    }();
    callback_->Process(message);

    EXPECT_EQ(testMessage_, future_.get());
}
}  // namespace ottest

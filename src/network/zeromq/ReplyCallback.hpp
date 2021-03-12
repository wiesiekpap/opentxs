// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"

namespace opentxs::network::zeromq::implementation
{
class ReplyCallback : virtual public zeromq::ReplyCallback
{
public:
    auto Process(const zeromq::Message& message) const -> OTZMQMessage override;

    ~ReplyCallback() override;

private:
    friend zeromq::ReplyCallback;

    const zeromq::ReplyCallback::ReceiveCallback callback_;

    auto clone() const -> ReplyCallback* override;

    ReplyCallback(zeromq::ReplyCallback::ReceiveCallback callback);
    ReplyCallback() = delete;
    ReplyCallback(const ReplyCallback&) = delete;
    ReplyCallback(ReplyCallback&&) = delete;
    auto operator=(const ReplyCallback&) -> ReplyCallback& = delete;
    auto operator=(ReplyCallback&&) -> ReplyCallback& = delete;
};
}  // namespace opentxs::network::zeromq::implementation

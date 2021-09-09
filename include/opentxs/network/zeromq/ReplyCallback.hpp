// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_HPP
#define OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/Pimpl.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class ReplyCallback;
}  // namespace zeromq
}  // namespace network

using OTZMQReplyCallback = Pimpl<network::zeromq::ReplyCallback>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
class OPENTXS_EXPORT ReplyCallback
{
public:
    using ReceiveCallback = std::function<OTZMQMessage(const Message&)>;

    static auto Factory(ReceiveCallback callback) -> OTZMQReplyCallback;

    virtual auto Process(const Message& message) const
        -> Pimpl<opentxs::network::zeromq::Message> = 0;

    virtual ~ReplyCallback() = default;

protected:
    ReplyCallback() = default;

private:
    friend OTZMQReplyCallback;

    virtual auto clone() const -> ReplyCallback* = 0;

    ReplyCallback(const ReplyCallback&) = delete;
    ReplyCallback(ReplyCallback&&) = default;
    auto operator=(const ReplyCallback&) -> ReplyCallback& = delete;
    auto operator=(ReplyCallback&&) -> ReplyCallback& = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

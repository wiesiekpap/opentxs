// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/zeromq/curve/Server.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQPublishSocket = Pimpl<network::zeromq::socket::Publish>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class OPENTXS_EXPORT Publish : virtual public curve::Server,
                               virtual public Sender
{
public:
    ~Publish() override = default;

protected:
    Publish() noexcept = default;

private:
    friend OTZMQPublishSocket;

    virtual auto clone() const noexcept -> Publish* = 0;

    Publish(const Publish&) = delete;
    Publish(Publish&&) = delete;
    auto operator=(const Publish&) -> Publish& = delete;
    auto operator=(Publish&&) -> Publish& = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

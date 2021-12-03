// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class OPENTXS_EXPORT Sender : virtual public Socket
{
public:
    virtual auto Send(Message&& message) const noexcept -> bool = 0;

    ~Sender() override = default;

protected:
    Sender() = default;

private:
    Sender(const Sender&) = delete;
    Sender(Sender&&) = delete;
    auto operator=(const Sender&) -> Sender& = delete;
    auto operator=(Sender&&) -> Sender& = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

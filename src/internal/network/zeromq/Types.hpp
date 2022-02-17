// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <future>
#include <tuple>

#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
using BatchID = std::size_t;
using SocketID = std::size_t;
using ReceiveCallback = std::function<void(Message&&)>;
using ModifyCallback = std::function<void(socket::Raw&)>;
using ThreadStartArgs = Vector<std::pair<socket::Raw*, ReceiveCallback>>;
using StartArgs = Vector<std::tuple<SocketID, socket::Raw*, ReceiveCallback>>;
using AsyncResult = std::pair<bool, std::future<bool>>;
using EndpointArg = std::pair<CString, socket::Direction>;
using EndpointArgs = Vector<EndpointArg>;

auto GetBatchID() noexcept -> BatchID;
auto GetSocketID() noexcept -> SocketID;
}  // namespace opentxs::network::zeromq

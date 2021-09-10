// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_REQUEST_TPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_REQUEST_TPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
template <typename Input>
auto Request::Send(const Input& data) const noexcept -> std::
    pair<opentxs::SendResult, opentxs::Pimpl<opentxs::network::zeromq::Message>>
{
    return send_request(Context().Message(data));
}
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_REPLY_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_REPLY_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/curve/Server.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Reply>::Pimpl(opentxs::network::zeromq::socket::Reply const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Reply>::operator opentxs::network::zeromq::socket::Reply&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Reply>::operator const opentxs::network::zeromq::socket::Reply &;
%rename(assign) operator=(const opentxs::network::zeromq::socket::Reply&);
%rename(ZMQReply) opentxs::network::zeromq::socket::Reply;
%template(OTZMQReplySocket) opentxs::Pimpl<opentxs::network::zeromq::socket::Reply>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Reply;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQReplySocket = Pimpl<network::zeromq::socket::Reply>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Reply : virtual public curve::Server
{
public:
    OPENTXS_EXPORT ~Reply() override = default;

protected:
    Reply() noexcept = default;

private:
    friend OTZMQReplySocket;

#ifdef _WIN32
public:
#endif
    OPENTXS_EXPORT virtual Reply* clone() const noexcept = 0;
#ifdef _WIN32
private:
#endif

    Reply(const Reply&) = delete;
    Reply(Reply&&) = delete;
    Reply& operator=(const Reply&) = delete;
    Reply& operator=(Reply&&) = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

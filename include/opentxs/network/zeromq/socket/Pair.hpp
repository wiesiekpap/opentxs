// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_PAIR_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_PAIR_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Pair>::Pimpl(opentxs::network::zeromq::socket::Pair const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Pair>::operator opentxs::network::zeromq::socket::Pair&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Pair>::operator const opentxs::network::zeromq::socket::Pair &;
%rename(assign) operator=(const opentxs::network::zeromq::socket::Pair&);
%rename(ZMQPair) opentxs::network::zeromq::socket::Pair;
%template(OTZMQPairSocket) opentxs::Pimpl<opentxs::network::zeromq::socket::Pair>;
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
class Pair;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQPairSocket = Pimpl<network::zeromq::socket::Pair>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Pair : virtual public socket::Socket, virtual public Sender
{
public:
    OPENTXS_EXPORT virtual const std::string& Endpoint() const noexcept = 0;

    OPENTXS_EXPORT ~Pair() override = default;

protected:
    Pair() noexcept = default;

private:
    friend OTZMQPairSocket;

    virtual Pair* clone() const noexcept = 0;

    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    Pair& operator=(const Pair&) = delete;
    Pair& operator=(Pair&&) = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_SUBSCRIBE_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_SUBSCRIBE_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/curve/Client.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Subscribe>::Pimpl(opentxs::network::zeromq::socket::Subscribe const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Subscribe>::operator opentxs::network::zeromq::socket::Subscribe&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Subscribe>::operator const opentxs::network::zeromq::socket::Subscribe &;
%rename(assign) operator=(const opentxs::network::zeromq::socket::Subscribe&);
%rename(ZMQSubscribe) opentxs::network::zeromq::socket::Subscribe;
%template(OTZMQSubscribeSocket) opentxs::Pimpl<opentxs::network::zeromq::socket::Subscribe>;
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
class Subscribe;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQSubscribeSocket = Pimpl<network::zeromq::socket::Subscribe>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Subscribe : virtual public curve::Client
{
public:
    OPENTXS_EXPORT virtual bool SetSocksProxy(
        const std::string& proxy) const noexcept = 0;

    OPENTXS_EXPORT ~Subscribe() override = default;

protected:
    Subscribe() noexcept = default;

private:
    friend OTZMQSubscribeSocket;

    virtual Subscribe* clone() const noexcept = 0;

    Subscribe(const Subscribe&) = delete;
    Subscribe(Subscribe&&) = delete;
    Subscribe& operator=(const Subscribe&) = delete;
    Subscribe& operator=(Subscribe&&) = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

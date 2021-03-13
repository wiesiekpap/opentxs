// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_REQUEST_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_REQUEST_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/curve/Client.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Request>::Pimpl(opentxs::network::zeromq::socket::Request const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Request>::operator opentxs::network::zeromq::socket::Request&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Request>::operator const opentxs::network::zeromq::socket::Request &;
%rename(assign) operator=(const opentxs::network::zeromq::socket::Request&);
%rename(ZMQRequest) opentxs::network::zeromq::socket::Request;
%template(OTZMQRequestSocket) opentxs::Pimpl<opentxs::network::zeromq::socket::Request>;
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
class Request;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQRequestSocket = Pimpl<network::zeromq::socket::Request>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Request : virtual public curve::Client
{
public:
    OPENTXS_EXPORT std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>
    Send(opentxs::Pimpl<opentxs::network::zeromq::Message>& message)
        const noexcept
    {
        return send_request(message.get());
    }
    OPENTXS_EXPORT std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>
    Send(Message& message) const noexcept
    {
        return send_request(message);
    }
    template <typename Input>
    OPENTXS_EXPORT std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>
    Send(const Input& data) const noexcept;
    OPENTXS_EXPORT virtual bool SetSocksProxy(
        const std::string& proxy) const noexcept = 0;

    OPENTXS_EXPORT ~Request() override = default;

protected:
    Request() noexcept = default;

private:
    friend OTZMQRequestSocket;

    virtual Request* clone() const noexcept = 0;
    virtual SendResult send_request(Message& message) const noexcept = 0;

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    Request& operator=(const Request&) = delete;
    Request& operator=(Request&&) = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

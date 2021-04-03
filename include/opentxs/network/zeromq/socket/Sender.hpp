// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_SEND_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_SEND_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

#ifdef SWIG
// clang-format off
%rename(ZMQSender) opentxs::network::zeromq::socket::Sender;
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
class Sender : virtual public Socket
{
public:
    OPENTXS_EXPORT bool Send(opentxs::Pimpl<opentxs::network::zeromq::Message>&
                                 message) const noexcept
    {
        return send(message.get());
    }
    OPENTXS_EXPORT bool Send(Message& message) const noexcept
    {
        return send(message);
    }
    template <typename Input>
    OPENTXS_EXPORT bool Send(const Input& data) const noexcept;

    OPENTXS_EXPORT ~Sender() override = default;

protected:
    Sender() = default;

private:
    virtual bool send(Message& message) const noexcept = 0;

    Sender(const Sender&) = delete;
    Sender(Sender&&) = delete;
    Sender& operator=(const Sender&) = delete;
    Sender& operator=(Sender&&) = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

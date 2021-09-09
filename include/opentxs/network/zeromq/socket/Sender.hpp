// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_SEND_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_SEND_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

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
    auto Send(opentxs::Pimpl<opentxs::network::zeromq::Message>& message)
        const noexcept -> bool
    {
        return send(message.get());
    }
    auto Send(Message& message) const noexcept -> bool { return send(message); }
    template <typename Input>
    auto Send(const Input& data) const noexcept -> bool;

    ~Sender() override = default;

protected:
    Sender() = default;

private:
    virtual auto send(Message& message) const noexcept -> bool = 0;

    Sender(const Sender&) = delete;
    Sender(Sender&&) = delete;
    auto operator=(const Sender&) -> Sender& = delete;
    auto operator=(Sender&&) -> Sender& = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

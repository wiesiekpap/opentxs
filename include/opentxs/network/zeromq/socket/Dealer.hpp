// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_DEALER_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_DEALER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/curve/Client.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Dealer>::Pimpl(opentxs::network::zeromq::socket::Dealer const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Dealer>::operator opentxs::network::zeromq::socket::Dealer&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Dealer>::operator const opentxs::network::zeromq::socket::Dealer &;
%rename(assign) operator=(const opentxs::network::zeromq::socket::Dealer&);
%rename(ZMQDealer) opentxs::network::zeromq::socket::Dealer;
%template(OTZMQDealerSocket) opentxs::Pimpl<opentxs::network::zeromq::socket::Dealer>;
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
class Dealer;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQDealerSocket = Pimpl<network::zeromq::socket::Dealer>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Dealer : virtual public curve::Client, virtual public Sender
{
public:
    OPENTXS_EXPORT virtual bool SetSocksProxy(
        const std::string& proxy) const noexcept = 0;

    OPENTXS_EXPORT ~Dealer() override = default;

protected:
    Dealer() noexcept = default;

private:
    friend OTZMQDealerSocket;

    virtual Dealer* clone() const noexcept = 0;

    Dealer(const Dealer&) = delete;
    Dealer(Dealer&&) = delete;
    Dealer& operator=(const Dealer&) = delete;
    Dealer& operator=(Dealer&&) = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_CURVE_SERVER_HPP
#define OPENTXS_NETWORK_ZEROMQ_CURVE_SERVER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/network/zeromq/socket/Socket.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::curve::Server::SetPrivateKey;
%interface(opentxs::network::zeromq::curve::Server);
// clang-format on
#endif  // SWIG

namespace opentxs
{
class Secret;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace curve
{
class Server : virtual public socket::Socket
{
public:
    OPENTXS_EXPORT virtual bool SetDomain(
        const std::string& domain) const noexcept = 0;
    OPENTXS_EXPORT virtual bool SetPrivateKey(
        const Secret& key) const noexcept = 0;
    OPENTXS_EXPORT virtual bool SetPrivateKey(
        const std::string& z85) const noexcept = 0;

    OPENTXS_EXPORT ~Server() override = default;

protected:
    Server() noexcept = default;

private:
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;
};
}  // namespace curve
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

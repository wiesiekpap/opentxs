// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_CURVE_CLIENT_HPP
#define OPENTXS_NETWORK_ZEROMQ_CURVE_CLIENT_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <tuple>

#include "opentxs/network/zeromq/socket/Socket.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::curve::Client::SetServerPubkey;
%template(CurveKeypair) std::pair<std::string, std::string>;
%interface(opentxs::network::zeromq::curve::Client);
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace curve
{
class Client : virtual public socket::Socket
{
public:
    OPENTXS_EXPORT static std::pair<std::string, std::string>
    RandomKeypair() noexcept;

    OPENTXS_EXPORT virtual bool SetKeysZ85(
        const std::string& serverPublic,
        const std::string& clientPrivate,
        const std::string& clientPublic) const noexcept = 0;
    OPENTXS_EXPORT virtual bool SetServerPubkey(
        const contract::Server& contract) const noexcept = 0;
    OPENTXS_EXPORT virtual bool SetServerPubkey(
        const Data& key) const noexcept = 0;

    OPENTXS_EXPORT ~Client() override = default;

protected:
    OPENTXS_EXPORT Client() noexcept = default;

private:
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) = delete;
};
}  // namespace curve
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

#endif

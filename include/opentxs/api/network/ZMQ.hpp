// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_NETWORK_ZMQ_HPP
#define OPENTXS_API_NETWORK_ZMQ_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Types.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq

class ServerConnection;
}  // namespace network

class Flag;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace network
{
class OPENTXS_EXPORT ZMQ
{
public:
    virtual auto Context() const
        -> const opentxs::network::zeromq::Context& = 0;
    virtual auto DefaultAddressType() const -> core::AddressType = 0;
    virtual auto KeepAlive() const -> std::chrono::seconds = 0;
    virtual void KeepAlive(const std::chrono::seconds duration) const = 0;
    virtual auto Linger() const -> std::chrono::seconds = 0;
    virtual auto ReceiveTimeout() const -> std::chrono::seconds = 0;
    virtual auto Running() const -> const Flag& = 0;
    virtual void RefreshConfig() const = 0;
    virtual auto SendTimeout() const -> std::chrono::seconds = 0;
    virtual auto Server(const std::string& id) const
        -> opentxs::network::ServerConnection& = 0;
    virtual auto SetSocksProxy(const std::string& proxy) const -> bool = 0;
    virtual auto SocksProxy() const -> std::string = 0;
    virtual auto SocksProxy(std::string& proxy) const -> bool = 0;
    virtual auto Status(const std::string& server) const -> ConnectionState = 0;

    OPENTXS_NO_EXPORT virtual ~ZMQ() = default;

protected:
    ZMQ() = default;

private:
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    auto operator=(const ZMQ&) -> ZMQ& = delete;
    auto operator=(const ZMQ&&) -> ZMQ& = delete;
};
}  // namespace network
}  // namespace api
}  // namespace opentxs
#endif

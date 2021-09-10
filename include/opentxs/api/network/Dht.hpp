// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_NETWORK_DHT_HPP
#define OPENTXS_API_NETWORK_DHT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>
#include <map>
#include <string>

#include "opentxs/identity/Nym.hpp"

namespace opentxs
{
namespace network
{
class OpenDHT;
}  // namespace network

namespace proto
{
class ServerContract;
class UnitDefinition;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace network
{
class OPENTXS_EXPORT Dht
{
public:
    enum class Callback : std::uint8_t {
        SERVER_CONTRACT = 0,
        ASSET_CONTRACT = 1,
        PUBLIC_NYM = 2
    };

    using NotifyCB = std::function<void(const std::string)>;
    using CallbackMap = std::map<Callback, NotifyCB>;

    virtual void GetPublicNym(const std::string& key) const = 0;
    virtual void GetServerContract(const std::string& key) const = 0;
    virtual void GetUnitDefinition(const std::string& key) const = 0;
    virtual void Insert(const std::string& key, const std::string& value)
        const = 0;
    virtual void Insert(const identity::Nym::Serialized& nym) const = 0;
    virtual void Insert(const proto::ServerContract& contract) const = 0;
    virtual void Insert(const proto::UnitDefinition& contract) const = 0;
    virtual auto OpenDHT() const -> const opentxs::network::OpenDHT& = 0;
    virtual void RegisterCallbacks(const CallbackMap& callbacks) const = 0;

    OPENTXS_NO_EXPORT virtual ~Dht() = default;

protected:
    Dht() = default;

private:
    Dht(const Dht&) = delete;
    Dht(Dht&&) = delete;
    auto operator=(const Dht&) -> Dht& = delete;
    auto operator=(Dht&&) -> Dht& = delete;
};
}  // namespace network
}  // namespace api
}  // namespace opentxs

#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

#include "network/DhtConfig.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/network/Dht.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/network/OpenDHT.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"

namespace opentxs
{
namespace api
{
class Core;
class Endpoints;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class Nym;
class ServerContract;
class UnitDefinition;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::api::network::implementation
{
class Dht final : virtual public opentxs::api::network::Dht
{
public:
    void GetPublicNym(const std::string& key) const final;
    void GetServerContract(const std::string& key) const final;
    void GetUnitDefinition(const std::string& key) const final;
    void Insert(const std::string& key, const std::string& value) const final;
    void Insert(const proto::Nym& nym) const final;
    void Insert(const proto::ServerContract& contract) const final;
    void Insert(const proto::UnitDefinition& contract) const final;
    auto OpenDHT() const -> const opentxs::network::OpenDHT& final;
    void RegisterCallbacks(const CallbackMap& callbacks) const final;

    Dht(const api::Core& api,
        const opentxs::network::zeromq::Context& zeromq,
        const api::Endpoints& endpoints,
        opentxs::network::DhtConfig& config) noexcept;

    ~Dht() final = default;

private:
    const api::Core& api_;
    mutable CallbackMap callback_map_;
    const opentxs::network::DhtConfig config_;
    std::unique_ptr<opentxs::network::OpenDHT> node_;
    OTZMQReplyCallback request_nym_callback_;
    OTZMQReplySocket request_nym_socket_;
    OTZMQReplyCallback request_server_callback_;
    OTZMQReplySocket request_server_socket_;
    OTZMQReplyCallback request_unit_callback_;
    OTZMQReplySocket request_unit_socket_;

    static auto ProcessPublicNym(
        const api::Core& api,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB) -> bool;
    static auto ProcessServerContract(
        const api::Core& api,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB) -> bool;
    static auto ProcessUnitDefinition(
        const api::Core& api,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB) -> bool;

    auto process_request(
        const opentxs::network::zeromq::Message& incoming,
        void (Dht::*get)(const std::string&) const) const -> OTZMQMessage;

    Dht() = delete;
    Dht(const Dht&) = delete;
    Dht(Dht&&) = delete;
    auto operator=(const Dht&) -> Dht& = delete;
    auto operator=(Dht&&) -> Dht& = delete;
};
}  // namespace opentxs::api::network::implementation

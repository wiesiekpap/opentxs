// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "api/network/Dht.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Proto.tpp"
#include "internal/api/network/Factory.hpp"
#include "internal/network/Factory.hpp"
#include "network/DhtConfig.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/protobuf/Nym.pb.h"
#include "opentxs/protobuf/ServerContract.pb.h"
#include "opentxs/protobuf/UnitDefinition.pb.h"

#define OT_METHOD "opentxs::api::network::implementation::Dht::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
using ReturnType = api::network::implementation::Dht;

auto DhtAPI(
    const api::Core& api,
    const opentxs::network::zeromq::Context& zeromq,
    const api::Endpoints& endpoints,
    const bool defaultEnable,
    std::int64_t& nymPublishInterval,
    std::int64_t& nymRefreshInterval,
    std::int64_t& serverPublishInterval,
    std::int64_t& serverRefreshInterval,
    std::int64_t& unitPublishInterval,
    std::int64_t& unitRefreshInterval) noexcept
    -> std::unique_ptr<api::network::Dht>
{
    auto config = network::DhtConfig{};
    auto notUsed{false};
    api.Config().CheckSet_bool(
        String::Factory("OpenDHT"),
        String::Factory("enable_dht"),
        defaultEnable,
        config.enable_dht_,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("nym_publish_interval"),
        config.nym_publish_interval_,
        nymPublishInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("nym_refresh_interval"),
        config.nym_refresh_interval_,
        nymRefreshInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("server_publish_interval"),
        config.server_publish_interval_,
        serverPublishInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("server_refresh_interval"),
        config.server_refresh_interval_,
        serverRefreshInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("unit_publish_interval"),
        config.unit_publish_interval_,
        unitPublishInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("unit_refresh_interval"),
        config.unit_refresh_interval_,
        unitRefreshInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("listen_port"),
        config.default_port_ + api.Instance(),
        config.listen_port_,
        notUsed);
    api.Config().CheckSet_str(
        String::Factory("OpenDHT"),
        String::Factory("bootstrap_url"),
        String::Factory(config.bootstrap_url_),
        config.bootstrap_url_,
        notUsed);
    api.Config().CheckSet_str(
        String::Factory("OpenDHT"),
        String::Factory("bootstrap_port"),
        String::Factory(config.bootstrap_port_),
        config.bootstrap_port_,
        notUsed);

    return std::make_unique<ReturnType>(api, zeromq, endpoints, config);
}
}  // namespace opentxs::factory

namespace opentxs::api::network::implementation
{
Dht::Dht(
    const api::Core& api,
    const opentxs::network::zeromq::Context& zeromq,
    const api::Endpoints& endpoints,
    opentxs::network::DhtConfig& config) noexcept
    : api_(api)
    , callback_map_()
    , config_(config)
    , node_(factory::OpenDHT(config_))
    , request_nym_callback_{zmq::ReplyCallback::Factory(
          [=](const zmq::Message& incoming) -> OTZMQMessage {
              return this->process_request(incoming, &Dht::GetPublicNym);
          })}
    , request_nym_socket_{zeromq.ReplySocket(
          request_nym_callback_,
          zmq::socket::Socket::Direction::Bind)}
    , request_server_callback_{zmq::ReplyCallback::Factory(
          [=](const zmq::Message& incoming) -> OTZMQMessage {
              return this->process_request(incoming, &Dht::GetServerContract);
          })}
    , request_server_socket_{zeromq.ReplySocket(
          request_server_callback_,
          zmq::socket::Socket::Direction::Bind)}
    , request_unit_callback_{zmq::ReplyCallback::Factory(
          [=](const zmq::Message& incoming) -> OTZMQMessage {
              return this->process_request(incoming, &Dht::GetUnitDefinition);
          })}
    , request_unit_socket_{zeromq.ReplySocket(
          request_unit_callback_,
          zmq::socket::Socket::Direction::Bind)}
{
    request_nym_socket_->Start(endpoints.DhtRequestNym());
    request_server_socket_->Start(endpoints.DhtRequestServer());
    request_unit_socket_->Start(endpoints.DhtRequestUnit());
}

void Dht::Insert(const std::string& key, const std::string& value) const
{
    node_->Insert(key, value);
}

void Dht::Insert(const identity::Nym::Serialized& nym) const
{
    node_->Insert(nym.nymid(), proto::ToString(nym));
}

void Dht::Insert(const proto::ServerContract& contract) const
{
    node_->Insert(contract.id(), proto::ToString(contract));
}

void Dht::Insert(const proto::UnitDefinition& contract) const
{
    node_->Insert(contract.id(), proto::ToString(contract));
}

void Dht::GetPublicNym(const std::string& key) const
{
    auto it = callback_map_.find(Dht::Callback::PUBLIC_NYM);
    bool haveCB = (it != callback_map_.end());
    NotifyCB notifyCB;

    if (haveCB) { notifyCB = it->second; }

    DhtResultsCallback gcb(
        [this, notifyCB, key](const DhtResults& values) -> bool {
            return ProcessPublicNym(api_, key, values, notifyCB);
        });

    node_->Retrieve(key, gcb);
}

void Dht::GetServerContract(const std::string& key) const
{
    auto it = callback_map_.find(Dht::Callback::SERVER_CONTRACT);
    bool haveCB = (it != callback_map_.end());
    NotifyCB notifyCB;

    if (haveCB) { notifyCB = it->second; }

    DhtResultsCallback gcb(
        [this, notifyCB, key](const DhtResults& values) -> bool {
            return ProcessServerContract(api_, key, values, notifyCB);
        });

    node_->Retrieve(key, gcb);
}

void Dht::GetUnitDefinition(const std::string& key) const
{
    auto it = callback_map_.find(Dht::Callback::ASSET_CONTRACT);
    bool haveCB = (it != callback_map_.end());
    NotifyCB notifyCB;

    if (haveCB) { notifyCB = it->second; }

    DhtResultsCallback gcb(
        [this, notifyCB, key](const DhtResults& values) -> bool {
            return ProcessUnitDefinition(api_, key, values, notifyCB);
        });

    node_->Retrieve(key, gcb);
}

auto Dht::OpenDHT() const -> const opentxs::network::OpenDHT& { return *node_; }

auto Dht::process_request(
    const zmq::Message& incoming,
    void (Dht::*get)(const std::string&) const) const -> OTZMQMessage
{
    OT_ASSERT(nullptr != get)

    bool output{false};
    const auto body = incoming.Body();

    if (1 < body.size()) {
        const auto id = [&] {
            auto output = api_.Factory().Identifier();
            output->Assign(body.at(1).Bytes());

            return output;
        }();

        if (false == id->empty()) {
            output = true;
            (this->*get)(id->str());
        }
    }

    return api_.Network().ZeroMQ().Message(output);
}

auto Dht::ProcessPublicNym(
    const api::Core& api,
    const std::string key,
    const DhtResults& values,
    NotifyCB notifyCB) -> bool
{
    std::string theresult;
    bool foundData = false;
    bool foundValid = false;

    if (key.empty()) { return false; }

    for (const auto& it : values) {
        if (nullptr == it) { continue; }

        auto& data = *it;
        foundData = data.size() > 0;

        if (0 == data.size()) { continue; }

        auto publicNym = proto::Factory<proto::Nym>(data);

        if (key != publicNym.nymid()) { continue; }

        auto existing = api.Wallet().Nym(api.Factory().NymID(key));

        if (existing) {
            if (existing->Revision() >= publicNym.revision()) { continue; }
        }

        auto saved = api.Wallet().Nym(publicNym);

        if (!saved) { continue; }

        foundValid = true;

        LogDebug(OT_METHOD)(__func__)(": Saved nym: ")(key).Flush();

        if (notifyCB) { notifyCB(key); }
    }

    if (!foundValid) {
        LogVerbose(OT_METHOD)(__func__)(": Found results, but none are valid.")
            .Flush();
    }

    if (!foundData) {
        LogVerbose(OT_METHOD)(__func__)(": All results are empty.").Flush();
    }

    return foundData;
}

auto Dht::ProcessServerContract(
    const api::Core& api,
    const std::string key,
    const DhtResults& values,
    NotifyCB notifyCB) -> bool
{
    std::string theresult;
    bool foundData = false;
    bool foundValid = false;

    if (key.empty()) { return false; }

    for (const auto& it : values) {
        if (nullptr == it) { continue; }

        auto& data = *it;
        foundData = data.size() > 0;

        if (0 == data.size()) { continue; }

        auto contract = proto::Factory<proto::ServerContract>(data);

        if (key != contract.id()) { continue; }

        try {
            auto saved = api.Wallet().Server(contract);
        } catch (...) {
            continue;
        }

        LogDebug(OT_METHOD)(__func__)(": Saved contract: ")(key).Flush();
        foundValid = true;

        if (notifyCB) { notifyCB(key); }

        break;  // We only need the first valid result
    }

    if (!foundValid) {
        LogOutput(OT_METHOD)(__func__)(": Found results, but none are valid.")
            .Flush();
    }

    if (!foundData) {
        LogOutput(OT_METHOD)(__func__)(": All results are empty.").Flush();
    }

    return foundData;
}

auto Dht::ProcessUnitDefinition(
    const api::Core& api,
    const std::string key,
    const DhtResults& values,
    NotifyCB notifyCB) -> bool
{
    std::string theresult;
    bool foundData = false;
    bool foundValid = false;

    if (key.empty()) { return false; }

    for (const auto& it : values) {
        if (nullptr == it) { continue; }

        auto& data = *it;
        foundData = data.size() > 0;

        if (0 == data.size()) { continue; }

        auto contract = proto::Factory<proto::UnitDefinition>(data);

        if (key != contract.id()) { continue; }

        try {
            api.Wallet().UnitDefinition(contract);
        } catch (...) {

            continue;
        }

        LogDebug(OT_METHOD)(__func__)(": Saved unit definition: ")(key).Flush();
        foundValid = true;

        if (notifyCB) { notifyCB(key); }

        break;  // We only need the first valid result
    }

    if (!foundValid) {
        LogOutput(OT_METHOD)(__func__)(": Found results, but none are valid.")
            .Flush();
    }

    if (!foundData) {
        LogOutput(OT_METHOD)(__func__)(": All results are empty.").Flush();
    }

    return foundData;
}

void Dht::RegisterCallbacks(const CallbackMap& callbacks) const
{
    callback_map_ = callbacks;
}
}  // namespace opentxs::api::network::implementation

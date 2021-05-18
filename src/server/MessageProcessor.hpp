// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>

#include "Proto.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/protobuf/ServerRequest.pb.h"

namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network

namespace server
{
class Server;
}  // namespace server

class Flag;
class OTPassword;
class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace opentxs::server
{
class MessageProcessor final : Lockable
{
public:
    void DropIncoming(const int count) const;
    void DropOutgoing(const int count) const;

    void cleanup();
    void init(const bool inproc, const int port, const Secret& privkey);
    void Start();

    explicit MessageProcessor(
        Server& server,
        const PasswordPrompt& reason,
        const Flag& running);

    ~MessageProcessor() final;

private:
    Server& server_;
    const PasswordPrompt& reason_;
    const Flag& running_;
    OTZMQListenCallback frontend_callback_;
    OTZMQRouterSocket frontend_socket_;
    OTZMQReplyCallback backend_callback_;
    OTZMQReplySocket backend_socket_;
    OTZMQListenCallback internal_callback_;
    OTZMQDealerSocket internal_socket_;
    OTZMQListenCallback notification_callback_;
    OTZMQPullSocket notification_socket_;
    std::thread thread_;
    const std::string internal_endpoint_;
    mutable std::mutex counter_lock_;
    mutable int drop_incoming_{0};
    mutable int drop_outgoing_{0};
    // nym id, connection identifier
    std::map<OTIdentifier, OTData> active_connections_;
    mutable std::shared_mutex connection_map_lock_;

    static auto get_connection(const network::zeromq::Message& incoming)
        -> OTData;

    auto extract_proto(const network::zeromq::Frame& incoming) const
        -> proto::ServerRequest;

    void associate_connection(
        const identifier::Nym& nymID,
        const Data& connection);
    auto process_backend(const network::zeromq::Message& incoming)
        -> OTZMQMessage;
    auto process_command(
        const proto::ServerRequest& request,
        identifier::Nym& nymID) -> bool;
    void process_frontend(const network::zeromq::Message& incoming);
    void process_internal(const network::zeromq::Message& incoming);
    void process_legacy(
        const Data& id,
        const network::zeromq::Message& incoming);
    auto process_message(const std::string& messageString, std::string& reply)
        -> bool;
    void process_notification(const network::zeromq::Message& incoming);
    void process_proto(
        const Data& id,
        const network::zeromq::Message& incoming);
    auto query_connection(const identifier::Nym& nymID) -> OTData;
    void run();

    MessageProcessor() = delete;
};
}  // namespace opentxs::server

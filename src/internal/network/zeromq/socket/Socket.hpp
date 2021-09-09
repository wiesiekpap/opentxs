// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Dealer;
class Pair;
class Publish;
class Pull;
class Push;
class Reply;
class Request;
class Router;
class Subscribe;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
class Pipeline;
class ReplyCallback;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::factory
{
auto DealerSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback)
    -> std::unique_ptr<network::zeromq::socket::Dealer>;
auto PairSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback,
    const bool startThread) -> std::unique_ptr<network::zeromq::socket::Pair>;
auto PairSocket(
    const network::zeromq::ListenCallback& callback,
    const network::zeromq::socket::Pair& peer,
    const bool startThread) -> std::unique_ptr<network::zeromq::socket::Pair>;
auto PairSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback,
    const std::string& endpoint)
    -> std::unique_ptr<network::zeromq::socket::Pair>;
auto Pipeline(
    const api::Core& api,
    const network::zeromq::Context& context,
    std::function<void(network::zeromq::Message&)> callback)
    -> std::unique_ptr<opentxs::network::zeromq::Pipeline>;
auto PublishSocket(const network::zeromq::Context& context)
    -> std::unique_ptr<network::zeromq::socket::Publish>;
auto PullSocket(const network::zeromq::Context& context, const bool direction)
    -> std::unique_ptr<network::zeromq::socket::Pull>;
auto PullSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback)
    -> std::unique_ptr<network::zeromq::socket::Pull>;
auto PushSocket(const network::zeromq::Context& context, const bool direction)
    -> std::unique_ptr<network::zeromq::socket::Push>;
auto ReplySocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ReplyCallback& callback)
    -> std::unique_ptr<network::zeromq::socket::Reply>;
auto RequestSocket(const network::zeromq::Context& context)
    -> std::unique_ptr<network::zeromq::socket::Request>;
auto RouterSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback)
    -> std::unique_ptr<network::zeromq::socket::Router>;
auto SubscribeSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback)
    -> std::unique_ptr<network::zeromq::socket::Subscribe>;
}  // namespace opentxs::factory

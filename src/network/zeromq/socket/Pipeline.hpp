// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <zmq.h>
#include <atomic>
#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "util/Gatekeeper.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Thread;
}  // namespace internal

namespace socket
{
class Raw;
}  // namespace socket

class Context;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq
{
class Pipeline::Imp : virtual public internal::Pipeline
{
public:
    virtual auto Close() const noexcept -> bool { return false; }
    virtual auto ConnectDealer(const std::string&, std::function<Message(bool)>)
        const noexcept -> bool
    {
        return false;
    }
    virtual auto ConnectionIDDealer() const noexcept -> std::size_t
    {
        return {};
    }
    virtual auto ConnectionIDInternal() const noexcept -> std::size_t
    {
        return {};
    }
    virtual auto ConnectionIDPull() const noexcept -> std::size_t { return {}; }
    virtual auto ConnectionIDSubscribe() const noexcept -> std::size_t
    {
        return {};
    }
    virtual auto PullFrom(const std::string&) const noexcept -> bool
    {
        return false;
    }
    virtual auto Push(Message&&) const noexcept -> bool { return false; }
    virtual auto Send(Message&&) const noexcept -> bool { return false; }
    virtual auto SubscribeTo(const std::string&) const noexcept -> bool
    {
        return false;
    }

    Imp() = default;

    ~Imp() override = default;

private:
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
class Pipeline final : virtual public zeromq::Pipeline::Imp
{
public:
    auto Close() const noexcept -> bool final;
    auto ConnectDealer(
        const std::string& endpoint,
        std::function<Message(bool)> notify) const noexcept -> bool final;
    auto ConnectionIDDealer() const noexcept -> std::size_t final;
    auto ConnectionIDInternal() const noexcept -> std::size_t final;
    auto ConnectionIDPull() const noexcept -> std::size_t final;
    auto ConnectionIDSubscribe() const noexcept -> std::size_t final;
    auto PullFrom(const std::string& endpoint) const noexcept -> bool final;
    auto Push(zeromq::Message&& data) const noexcept -> bool final;
    auto Send(zeromq::Message&& data) const noexcept -> bool final;
    auto SubscribeTo(const std::string& endpoint) const noexcept -> bool final;

    Pipeline(
        const api::Session& api,
        const zeromq::Context& context,
        std::function<void(zeromq::Message&&)> callback) noexcept;

    ~Pipeline() final;

private:
    const zeromq::Context& context_;
    mutable std::mutex to_internal_lock_;
    mutable std::mutex to_dealer_lock_;
    mutable Gatekeeper gate_;
    mutable std::atomic<bool> shutdown_;
    internal::Batch& batch_;
    socket::Raw& sub_;          // NOTE activated by SubscribeTo()
    socket::Raw& pull_;         // NOTE activated by PullFrom()
    socket::Raw& outgoing_;     // NOTE receives from to_dealer_
    socket::Raw& dealer_;       // NOTE activated by ConnectDealer()
    socket::Raw& internal_;     // NOTE receives from to_internal_
    socket::Raw& to_internal_;  // NOTE activated by Push()
    socket::Raw& to_dealer_;    // NOTE activated by Send()
    internal::Thread* thread_;

    auto connect(
        SocketID id,
        const std::string& endpoint,
        std::function<Message(bool)> notify = {}) const noexcept
        -> std::pair<bool, std::future<bool>>;

    Pipeline() = delete;
    Pipeline(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    auto operator=(const Pipeline&) -> Pipeline& = delete;
    auto operator=(Pipeline&&) -> Pipeline& = delete;
};
}  // namespace opentxs::network::zeromq::implementation

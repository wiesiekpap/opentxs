// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/ThreadPool.cpp"

#pragma once

#include "opentxs/api/ThreadPool.hpp"  // IWYU pragma: associated

#include <atomic>
#include <map>
#include <mutex>
#include <vector>

#include "internal/api/Api.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::implementation
{
class ThreadPool final : public internal::ThreadPool
{
public:
    auto Endpoint() const noexcept -> std::string final;
    auto Register(WorkType type, Callback handler) const noexcept -> bool final;

    auto Shutdown() noexcept -> void final;

    ThreadPool(const opentxs::network::zeromq::Context& zmq) noexcept;

    ~ThreadPool() final = default;

private:
    using Map = std::map<WorkType, Callback>;
    using Vector = std::vector<OTZMQPullSocket>;

    const opentxs::network::zeromq::Context& zmq_;
    mutable std::mutex lock_;
    mutable Map map_;
    std::atomic<bool> running_;
    OTZMQListenCallback cbi_;
    Vector workers_;
    OTZMQPushSocket int_;
    OTZMQListenCallback cbe_;
    OTZMQPullSocket ext_;

    auto callback(zmq::Message& in) noexcept -> void;

    ThreadPool() = delete;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    auto operator=(const ThreadPool&) -> ThreadPool& = delete;
    auto operator=(ThreadPool&&) -> ThreadPool& = delete;
};
}  // namespace opentxs::api::implementation

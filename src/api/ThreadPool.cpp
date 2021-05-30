// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"        // IWYU pragma: associated
#include "1_Internal.hpp"      // IWYU pragma: associated
#include "api/ThreadPool.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

#include "internal/api/Factory.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

#define OT_METHOD "opentxs::api::implementation::ThreadPool::"

namespace opentxs::factory
{
using ReturnType = api::implementation::ThreadPool;

auto ThreadPool(const zmq::Context& zmq) noexcept
    -> std::unique_ptr<api::internal::ThreadPool>
{
    return std::make_unique<ReturnType>(zmq);
}
}  // namespace opentxs::factory

namespace opentxs::api
{
auto ThreadPool::Capacity() noexcept -> std::size_t
{
    return std::thread::hardware_concurrency();
}

auto ThreadPool::MakeWork(const zmq::Context& zmq, WorkType type) noexcept
    -> OTZMQMessage
{
    auto work = zmq.Message(type);
    work->StartBody();

    return work;
}
}  // namespace opentxs::api

namespace opentxs::api::implementation
{
constexpr auto endpoint_{"inproc://opentxs//thread_pool/1"};
constexpr auto internal_endpoint_{"inproc://opentxs//thread_pool/internal"};
using Direction = zmq::socket::Socket::Direction;

ThreadPool::ThreadPool(const zmq::Context& zmq) noexcept
    : zmq_(zmq)
    , lock_()
    , map_()
    , running_(true)
    , cbi_(zmq::ListenCallback::Factory([this](auto& in) { callback(in); }))
    , workers_([&] {
        auto out = Vector{};
        const auto target = Capacity();
        out.reserve(target);

        for (unsigned int i{0}; i < target; ++i) {
            auto& worker =
                out.emplace_back(zmq_.PullSocket(cbi_, Direction::Connect));
            const auto rc = worker->Start(internal_endpoint_);

            OT_ASSERT(rc);

            LogTrace("Started thread pool worker #")(i).Flush();
        }

        return out;
    }())
    , int_([&] {
        auto out = zmq_.PushSocket(Direction::Bind);
        const auto rc = out->Start(internal_endpoint_);

        OT_ASSERT(rc);

        return out;
    }())
    , cbe_(zmq::ListenCallback::Factory([this](auto& in) { int_->Send(in); }))
    , ext_([&] {
        auto out = zmq_.PullSocket(cbe_, Direction::Bind);
        const auto rc = out->Start(endpoint_);

        OT_ASSERT(rc);

        return out;
    }())
{
}

auto ThreadPool::callback(zmq::Message& in) noexcept -> void
{
    const auto header = in.Header();

    try {
        const auto size = header.size();

        if (1u > size) { throw std::runtime_error{"Missing type frame"}; }

        const auto& workFrame = header.at(size - 1u);
        const auto type = [&] {
            try {

                return workFrame.as<WorkType>();
            } catch (...) {

                OT_FAIL;
            }
        }();
        const auto& cb = [&] {
            auto lock = Lock{lock_};

            try {

                return map_.at(type);
            } catch (...) {
                throw std::runtime_error{"No callback for specified work type"};
            }
        }();

        cb(in);
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        return;
    }
}

auto ThreadPool::Endpoint() const noexcept -> std::string { return endpoint_; }

auto ThreadPool::Register(WorkType type, Callback handler) const noexcept
    -> bool
{
    if (!handler) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": invalid handler").Flush();

        return false;
    }

    auto lock = Lock{lock_};
    const auto [it, added] = map_.try_emplace(type, std::move(handler));

    return added;
}

auto ThreadPool::Shutdown() noexcept -> void
{
    if (running_.exchange(false)) {
        ext_->Close();
        int_->Close();
        workers_.clear();
    }
}
}  // namespace opentxs::api::implementation

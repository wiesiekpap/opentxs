// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <future>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Future.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Gatekeeper.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
template <typename CRTP, typename JobType>
class Actor
{
private:
    std::promise<void> init_promise_;
    std::shared_future<void> init_future_;
    std::atomic<bool> running_;

public:
    using Message = network::zeromq::Message;

protected:
    using Work = JobType;

    Gatekeeper worker_gate_;
    network::zeromq::Pipeline pipeline_;

    auto trigger() const noexcept -> void
    {
        auto shutdown = worker_gate_.get();

        if (shutdown) { return; }

        const auto running = state_machine_queued_.exchange(true);

        if (false == running) {
            pipeline_.Push(MakeWork(OT_ZMQ_STATE_MACHINE_SIGNAL));
        }
    }
    auto signal_shutdown() const noexcept -> std::shared_future<void>
    {
        if (running_) { pipeline_.Push(MakeWork(WorkType::Shutdown)); }

        return shutdown_;
    }
    auto signal_startup() const noexcept -> void
    {
        if (running_) { pipeline_.Push(MakeWork(OT_ZMQ_INIT_SIGNAL)); }
    }
    auto wait_for_init() const noexcept -> void { init_future_.get(); }

    auto do_work() noexcept
    {
        rate_limit_state_machine();
        state_machine_queued_.store(false);
        repeat(downcast().state_machine());
    }
    auto init_complete() noexcept -> void { init_promise_.set_value(); }

    Actor(
        const api::Session& api,
        const std::chrono::milliseconds rateLimit,
        const network::zeromq::EndpointArgs& subscribe = {},
        const network::zeromq::EndpointArgs& pull = {},
        const network::zeromq::EndpointArgs& dealer = {}) noexcept
        : init_promise_()
        , init_future_(init_promise_.get_future())
        , running_(true)
        , worker_gate_()
        , pipeline_(api.Network().ZeroMQ().Internal().Pipeline(
              [this](auto&& in) { this->worker(std::move(in)); },
              [&] {
                  using Direction = network::zeromq::socket::Direction;
                  auto out{subscribe};
                  out.emplace_back(
                      api.Endpoints().Shutdown(), Direction::Connect);

                  return out;
              }(),
              pull,
              dealer))
        , rate_limit_(rateLimit)
        , shutdown_promise_()
        , shutdown_(shutdown_promise_.get_future())
        , last_executed_(Clock::now())
        , state_machine_queued_(false)
    {
        LogTrace()(OT_PRETTY_CLASS())("using ZMQ batch ")(pipeline_.BatchID())
            .Flush();
    }

    ~Actor() { signal_shutdown().get(); }

private:
    const std::chrono::milliseconds rate_limit_;
    std::promise<void> shutdown_promise_;
    std::shared_future<void> shutdown_;
    Time last_executed_;
    mutable std::atomic<bool> state_machine_queued_;

    auto rate_limit_state_machine() const noexcept
    {
        const auto wait = std::chrono::duration_cast<std::chrono::microseconds>(
            rate_limit_ - (Clock::now() - last_executed_));

        if (0 < wait.count()) {
            LogInsane()(OT_PRETTY_CLASS())("rate limited for ")(wait.count())(
                " microseconds")
                .Flush();
            Sleep(wait);
        }
    }

    inline auto downcast() noexcept -> CRTP&
    {
        return static_cast<CRTP&>(*this);
    }
    auto repeat(const bool again) noexcept -> void
    {
        if (again) { trigger(); }

        last_executed_ = Clock::now();
    }
    auto shutdown() noexcept -> void
    {
        init_future_.get();

        if (auto previous = running_.exchange(false); previous) {
            downcast().do_shutdown();
            pipeline_.Close();
            shutdown_promise_.set_value();
        }
    }
    auto worker(network::zeromq::Message&& in) noexcept -> void
    {
        const auto& log = LogTrace();
        log(OT_PRETTY_CLASS())("Message received").Flush();
        const auto body = in.Body();

        if (1 > body.size()) {
            LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

            OT_FAIL;
        }

        const auto work = [&] {
            try {

                return body.at(0).as<Work>();
            } catch (...) {

                OT_FAIL;
            }
        }();
        const auto type = static_cast<OTZMQWorkType>(work);
        log(OT_PRETTY_CLASS())("message type is: ")(type).Flush();

        if (OT_ZMQ_INIT_SIGNAL == type) {
            log(OT_PRETTY_CLASS())("initializing").Flush();
            downcast().startup();

            try {
                init_promise_.set_value();
                log(OT_PRETTY_CLASS())("initialization complete").Flush();
            } catch (...) {
                LogError()(OT_PRETTY_CLASS())("init message received twice")
                    .Flush();

                OT_FAIL;
            }

            return;
        }

        // NOTE: do not process any messages until init is received
        if (false == IsReady(init_future_)) {
            log(OT_PRETTY_CLASS())("dropping message of type ")(
                type)(" until init is processed")
                .Flush();

            return;
        }

        {
            // NOTE do not process any messages after shutdown is ordered
            auto shutdown = worker_gate_.get();

            if (shutdown || (false == running_)) { return; }

            switch (type) {
                case value(WorkType::Shutdown): {
                    log(OT_PRETTY_CLASS())("shutting down").Flush();
                    this->shutdown();
                } break;
                case OT_ZMQ_STATE_MACHINE_SIGNAL: {
                    log(OT_PRETTY_CLASS())("executing state machine").Flush();
                    do_work();
                } break;
                default: {
                    log(OT_PRETTY_CLASS())("processing ")(type).Flush();
                    downcast().pipeline(work, std::move(in));
                }
            }
        }

        log(OT_PRETTY_CLASS())("message processing complete").Flush();

        if (false == running_) { worker_gate_.shutdown(); }
    }
};
}  // namespace opentxs

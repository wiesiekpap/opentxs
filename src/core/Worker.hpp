// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <functional>
#include <future>

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "util/Work.hpp"

namespace opentxs
{
template <typename Enum>
auto MakeWork(const api::Core& api, const Enum type) noexcept -> OTZMQMessage
{
    return api.Network().ZeroMQ().TaggedMessage(type);
}

template <typename Child, typename API = api::client::Manager>
class Worker
{
public:
    template <typename Enum>
    auto MakeWork(const Enum type) const noexcept -> OTZMQMessage
    {
        return opentxs::MakeWork(api_, type);
    }

protected:
    using Endpoints = std::vector<std::string>;

    const API& api_;
    const std::chrono::milliseconds rate_limit_;
    OTFlag running_;
    std::promise<void> shutdown_promise_;
    OTZMQPipeline pipeline_;

    auto trigger() const noexcept -> void
    {
        if (false == running_.get()) { return; }

        const auto running = state_machine_queued_.exchange(true);

        if (false == running) {
            auto work = MakeWork(OT_ZMQ_STATE_MACHINE_SIGNAL);
            pipeline_->Push(work);
        }
    }

    auto do_work() noexcept
    {
        rate_limit_state_machine();
        state_machine_queued_.store(false);
        repeat(downcast().state_machine());
    }
    auto init_executor(const Endpoints endpoints = {}) noexcept -> void
    {
        pipeline_->Start(api_.Endpoints().Shutdown());

        for (const auto& endpoint : endpoints) { pipeline_->Start(endpoint); }
    }
    auto stop_worker() noexcept -> std::shared_future<void>
    {
        pipeline_->Close();

        if (running_.get()) { downcast().shutdown(shutdown_promise_); }

        return shutdown_;
    }

    Worker(const API& api, const std::chrono::milliseconds rateLimit) noexcept
        : api_(api)
        , rate_limit_(rateLimit)
        , running_(Flag::Factory(true))
        , shutdown_promise_()
        , pipeline_(api.Factory().Pipeline(
              [this](auto& in) { downcast().pipeline(in); }))
        , shutdown_(shutdown_promise_.get_future())
        , last_executed_(Clock::now())
        , state_machine_queued_(false)
    {
    }

    ~Worker() { stop_worker().get(); }

private:
    std::shared_future<void> shutdown_;
    Time last_executed_;
    mutable std::atomic<bool> state_machine_queued_;

    auto rate_limit_state_machine() const noexcept
    {
        const auto wait = std::chrono::duration_cast<std::chrono::milliseconds>(
            rate_limit_ - (Clock::now() - last_executed_));

        if (0 < wait.count()) { Sleep(wait); }
    }

    inline auto downcast() noexcept -> Child&
    {
        return static_cast<Child&>(*this);
    }
    auto repeat(const bool again) noexcept -> void
    {
        if (again) { trigger(); }

        last_executed_ = Clock::now();
    }
};
}  // namespace opentxs

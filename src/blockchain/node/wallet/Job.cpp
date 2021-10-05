// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "blockchain/node/wallet/Job.hpp"  // IWYU pragma: associated

#include <chrono>
#include <functional>
#include <utility>

#include "blockchain/node/wallet/SubchainStateData.hpp"
#include "internal/api/network/Network.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "util/JobCounter.hpp"
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::blockchain::node::wallet::Job::"

namespace opentxs::blockchain::node::wallet
{
Job::Job(SubchainStateData& parent) noexcept
    : parent_(parent)
    , shutdown_(false)
    , lock_()
    , running_(0)
    , cv_()
{
}

auto Job::finish(Lock& lock) noexcept -> void
{
    --running_;

    OT_ASSERT(0 <= running_);

    lock.unlock();
    cv_.notify_all();
}

auto Job::IsRunning() const noexcept -> bool
{
    auto lock = Lock{lock_};

    return is_running(lock);
}

auto Job::is_running(Lock& lock) const noexcept -> bool
{
    static constexpr auto zero = std::chrono::seconds{0};
    cv_.wait_for(lock, zero);

    return (0 < running_);
}

auto Job::queue_work(
    SimpleCallback cb,
    const char* log,
    bool lockIsHeld) noexcept -> bool
{
    OT_ASSERT(cb);

    ++parent_.job_counter_;

    {
        auto lock = Lock{lock_, std::defer_lock};

        if (false == lockIsHeld) { lock.lock(); }

        ++running_;
    }

    const auto queued = parent_.api_.Network().Asio().Internal().PostCPU(
        [this, job = std::move(cb)] {
            auto post = ScopeGuard{[this] {
                parent_.task_finished_(parent_.db_key_, type());
                --parent_.job_counter_;
            }};

            job();
        });

    if (queued) {
        LogDebug(OT_METHOD)(__func__)(": ")(parent_.name_)(" ")(
            log)(" job queued")
            .Flush();
    } else {
        LogDebug(OT_METHOD)(__func__)(": ")(parent_.name_)(" failed to queue ")(
            log)(" job")
            .Flush();
        --parent_.job_counter_;
    }

    return queued;
}

auto Job::Shutdown() noexcept -> void { shutdown_ = true; }

auto Job::wait(Lock& lock) const noexcept -> void
{
    const auto start = Clock::now();
    static constexpr auto limit = std::chrono::seconds{10};

    while (true) {
        if (cv_.wait_for(lock, limit, [this] { return 0 == running_; })) {
            break;
        }

        const auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            Clock::now() - start);
        LogOutput(OT_METHOD)(__func__)(": ")(parent_.name_)(" waiting on ")(
            type())(" job for ")(duration.count())(" seconds")
            .Flush();
    }
}
}  // namespace opentxs::blockchain::node::wallet

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>
#include <list>
#include <memory>
#include <thread>
#include <tuple>

#include "internal/api/Api.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Periodic.hpp"
#include "opentxs/core/Lockable.hpp"

namespace opentxs
{
namespace api
{
namespace network
{
class Dht;
}  // namespace network

namespace storage
{
class Storage;
}  // namespace storage
}  // namespace api

class Flag;
}  // namespace opentxs

namespace opentxs::api::implementation
{
class Scheduler : virtual public api::Periodic, public Lockable
{
public:
    const api::internal::Context& parent_;
    std::int64_t nym_publish_interval_{0};
    std::int64_t nym_refresh_interval_{0};
    std::int64_t server_publish_interval_{0};
    std::int64_t server_refresh_interval_{0};
    std::int64_t unit_publish_interval_{0};
    std::int64_t unit_refresh_interval_{0};
    Flag& running_;

    auto Cancel(const int task) const -> bool final
    {
        return parent_.Cancel(task);
    }
    auto Reschedule(const int task, const std::chrono::seconds& interval) const
        -> bool final
    {
        return parent_.Reschedule(task, interval);
    }
    auto Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last) const -> int final
    {
        return parent_.Schedule(interval, task, last);
    }

    ~Scheduler() override;

protected:
    void Start(
        const api::storage::Storage* const storage,
        const api::network::Dht& dht);

    Scheduler(const api::internal::Context& parent, Flag& running);

private:
    std::thread periodic_;

    virtual void storage_gc_hook() = 0;

    Scheduler() = delete;
    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    auto operator=(const Scheduler&) -> Scheduler& = delete;
    auto operator=(Scheduler&&) -> Scheduler& = delete;

    void thread();
};
}  // namespace opentxs::api::implementation

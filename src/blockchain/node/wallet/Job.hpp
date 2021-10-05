// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs
{
namespace blockchain
{
namespace node
{
namespace wallet
{
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
class Job
{
public:
    auto IsRunning() const noexcept -> bool;
    virtual auto Reorg(const block::Position& parent) noexcept -> void = 0;
    virtual auto Run() noexcept -> bool = 0;
    auto Shutdown() noexcept -> void;

    Job(SubchainStateData& parent) noexcept;

    virtual ~Job() = default;

protected:
    SubchainStateData& parent_;
    std::atomic_bool shutdown_;
    mutable std::mutex lock_;

    // WARNING lock will be released and re-acquired
    auto is_running(Lock& lock) const noexcept -> bool;
    // WARNING lock will be released and re-acquired
    auto wait(Lock& lock) const noexcept -> void;

    auto finish(Lock& lock) noexcept -> void;
    auto queue_work(
        SimpleCallback cb,
        const char* log,
        bool lockIsHeld) noexcept -> bool;

private:
    long long int running_;
    mutable std::condition_variable cv_;

    virtual auto type() const noexcept -> const char* = 0;

    Job() = delete;
    Job(const Job&) = delete;
    Job(Job&&) = delete;
    auto operator=(const Job&) -> Job& = delete;
    auto operator=(Job&&) -> Job& = delete;
};
}  // namespace opentxs::blockchain::node::wallet

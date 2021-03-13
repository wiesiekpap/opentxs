// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

namespace opentxs
{
using OutstandingMap = std::map<int, std::atomic_int>;

class JobCounter;

class Outstanding
{
public:
    operator int() const noexcept { return position_.value()->second; }

    auto limited() const noexcept -> bool
    {
        static const auto limit =
            static_cast<int>(std::thread::hardware_concurrency());

        return position_.value()->second >= limit;
    }

    auto operator++() noexcept -> Outstanding&
    {
        ++(position_.value()->second);

        return *this;
    }
    auto operator--() noexcept -> Outstanding&
    {
        --(position_.value()->second);

        return *this;
    }

    Outstanding(JobCounter& parent, OutstandingMap::iterator position) noexcept;
    Outstanding(Outstanding&& rhs) noexcept;
    Outstanding(const Outstanding&) = delete;

    ~Outstanding();

private:
    JobCounter& parent_;
    std::optional<OutstandingMap::iterator> position_;
};

class JobCounter
{
public:
    auto Allocate() noexcept -> Outstanding;
    auto Deallocate(OutstandingMap::iterator position) noexcept -> void;

    JobCounter() noexcept;

private:
    mutable std::mutex lock_;
    int counter_;
    OutstandingMap map_;
};
}  // namespace opentxs

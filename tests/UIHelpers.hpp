// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"

namespace
{
struct Counter {
    std::atomic_int expected_{};
    std::atomic_int updated_{};
};

[[maybe_unused]] auto make_cb(Counter& counter, const std::string name) noexcept
{
    return [&counter, name]() {
        auto& [expected, value] = counter;

        if (++value > expected) { std::cout << name << ": " << value << '\n'; }
    };
}

[[maybe_unused]] auto wait_for_counter(
    Counter& data,
    const bool hard = true) noexcept -> bool
{
    const auto limit =
        hard ? std::chrono::seconds(300) : std::chrono::seconds(10);
    auto start = ot::Clock::now();
    auto& [expected, updated] = data;

    while ((updated < expected) && ((ot::Clock::now() - start) < limit)) {
        ot::Sleep(std::chrono::milliseconds(100));
    }

    if (false == hard) { updated.store(expected.load()); }

    return updated >= expected;
}
}  // namespace

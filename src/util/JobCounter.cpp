// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "util/JobCounter.hpp"  // IWYU pragma: associated

#include <chrono>

#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs
{
JobCounter::JobCounter() noexcept
    : lock_()
    , counter_()
    , map_()
{
}

auto JobCounter::Allocate() noexcept -> Outstanding
{
    auto lock = Lock{lock_};
    auto [it, added] = map_.emplace(++counter_, 0);

    OT_ASSERT(added);

    return Outstanding{*this, it};
}

auto JobCounter::Deallocate(OutstandingMap::iterator position) noexcept -> void
{
    auto lock = Lock{lock_};
    map_.erase(position);
}

Outstanding::Outstanding(
    JobCounter& parent,
    OutstandingMap::iterator position) noexcept
    : parent_(parent)
    , position_(position)
{
}

Outstanding::Outstanding(Outstanding&& rhs) noexcept
    : parent_(rhs.parent_)
    , position_(rhs.position_.value())
{
    rhs.position_ = std::nullopt;
}

Outstanding::~Outstanding()
{
    if (position_.has_value()) {
        auto& position = position_.value();
        auto& counter = position->second;

        while (0 < counter) { Sleep(std::chrono::microseconds(100)); }

        parent_.Deallocate(position);
        position_ = std::nullopt;
    }
}
}  // namespace opentxs

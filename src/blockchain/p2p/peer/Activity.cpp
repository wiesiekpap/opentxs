// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/p2p/peer/Peer.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::p2p::implementation
{
Peer::Activity::Activity() noexcept
    : lock_()
    , activity_(Clock::now())
{
}

auto Peer::Activity::Bump() noexcept -> void
{
    Lock lock(lock_);
    activity_ = Clock::now();
}

auto Peer::Activity::get() const noexcept -> Time
{
    Lock lock(lock_);

    return activity_;
}
}  // namespace opentxs::blockchain::p2p::implementation

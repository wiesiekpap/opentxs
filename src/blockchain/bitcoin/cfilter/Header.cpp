// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"  // IWYU pragma: associated

#include "core/FixedByteArray.tpp"

namespace opentxs::blockchain::cfilter
{
Header::Header() noexcept
    : FixedByteArray()
{
    static_assert(payload_size_ == 32u);
}

Header::Header(const ReadView bytes) noexcept(false)
    : FixedByteArray(bytes)
{
}

Header::Header(const Header& rhs) noexcept
    : FixedByteArray(rhs)
{
}

auto Header::operator=(const Header& rhs) noexcept -> Header&
{
    FixedByteArray::operator=(rhs);

    return *this;
}

Header::~Header() = default;
}  // namespace opentxs::blockchain::cfilter

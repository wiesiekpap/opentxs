// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::blockchain::cfilter
{
class Header : virtual public FixedByteArray<32>
{
public:
    Header() noexcept;
    Header(const ReadView bytes) noexcept(false);
    Header(const Header& rhs) noexcept;
    auto operator=(const Header& rhs) noexcept -> Header&;

    ~Header() override;
};
}  // namespace opentxs::blockchain::cfilter

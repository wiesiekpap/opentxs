// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/common/Base.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

namespace ottest
{
Base::Base() noexcept
    : ot_(ot::Context())
{
}
}  // namespace ottest

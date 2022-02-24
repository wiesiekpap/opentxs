// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"

namespace ot = opentxs;

namespace ottest
{
class Base
{
protected:
    const ot::api::Context& ot_;

    Base() noexcept
        : ot_(ot::Context())
    {
    }

    virtual ~Base() = default;
};
}  // namespace ottest

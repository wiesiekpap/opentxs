// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/Activity.hpp"

namespace opentxs::api::session::internal
{
class Activity : virtual public session::Activity
{
public:
    auto Internal() const noexcept -> const Activity& final { return *this; }

    auto Internal() noexcept -> Activity& final { return *this; }

    ~Activity() override = default;
};
}  // namespace opentxs::api::session::internal

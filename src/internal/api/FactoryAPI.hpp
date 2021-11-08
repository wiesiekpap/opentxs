// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/Factory.hpp"

namespace opentxs::api::internal
{
class Factory : virtual public api::Factory
{
public:
    auto Internal() const noexcept -> const Factory& final { return *this; }

    auto Internal() noexcept -> Factory& final { return *this; }

    ~Factory() override = default;
};
}  // namespace opentxs::api::internal

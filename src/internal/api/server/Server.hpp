// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/Api.hpp"
#include "opentxs/api/server/Manager.hpp"

namespace opentxs::api::server::internal
{
struct Manager : virtual public api::server::Manager,
                 virtual public api::internal::Core {

    ~Manager() override = default;
};
}  // namespace opentxs::api::server::internal

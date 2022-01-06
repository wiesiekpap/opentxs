// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/Endpoints.hpp"

namespace opentxs::api::session::internal
{
class Endpoints : virtual public api::session::Endpoints
{
public:
    virtual auto BlockchainBlockUpdated(const opentxs::blockchain::Type chain)
        const noexcept -> UnallocatedCString = 0;
    virtual auto BlockchainFilterUpdated(const opentxs::blockchain::Type chain)
        const noexcept -> UnallocatedCString = 0;
    auto Internal() const noexcept -> const Endpoints& final { return *this; }
    virtual auto ProcessPushNotification() const noexcept
        -> UnallocatedCString = 0;
    virtual auto PushNotification() const noexcept -> UnallocatedCString = 0;

    auto Internal() noexcept -> Endpoints& final { return *this; }

    ~Endpoints() override = default;
};
}  // namespace opentxs::api::session::internal

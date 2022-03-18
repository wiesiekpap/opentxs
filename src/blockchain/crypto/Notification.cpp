// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/crypto/Notification.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs::blockchain::crypto::implementation
{
Notification::Notification(
    const api::Session& api,
    const crypto::Account& parent,
    OTIdentifier&& id,
    OTIdentifier out) noexcept
    : Subaccount(api, parent, SubaccountType::Notification, std::move(id), out)
{
    init();
}

Notification::Notification(
    const api::Session& api,
    const crypto::Account& parent,
    OTIdentifier&& id) noexcept
    : Notification(api, parent, std::move(id), api.Factory().Identifier())
{
}
}  // namespace opentxs::blockchain::crypto::implementation

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "internal/api/client/Factory.hpp"  // IWYU pragma: associated

#include "api/client/ui/Imp-base.hpp"
#include "internal/api/client/Client.hpp"

namespace opentxs::factory
{
auto UI(
    const api::client::Manager& api,
    const api::client::internal::Blockchain& blockchain,
    const Flag& running) noexcept -> std::unique_ptr<api::client::internal::UI>
{
    using ReturnType = api::client::internal::UI;
    auto imp = std::make_unique<api::client::UI::Imp>(api, blockchain, running);

    return std::make_unique<ReturnType>(imp.release());
}
}  // namespace opentxs::factory

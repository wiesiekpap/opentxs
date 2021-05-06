// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "1_Internal.hpp"   // IWYU pragma: associated
#include "api/Context.hpp"  // IWYU pragma: associated

#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/response/Base.hpp"

// #define OT_METHOD "opentxs::api::implementation::Context::"

namespace opentxs::api::implementation
{
auto Context::RPC(const ReadView command, const AllocateOutput response)
    const noexcept -> bool
{
    return RPC(*rpc::request::Factory(command))->Serialize(response);
}
}  // namespace opentxs::api::implementation

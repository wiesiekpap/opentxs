// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blind/CashType.hpp"
// IWYU pragma: no_include "opentxs/blind/PurseType.hpp"
// IWYU pragma: no_include "opentxs/blind/TokenState.hpp"

#pragma once

#include "opentxs/blind/Types.hpp"
#include "serialization/protobuf/CashEnums.pb.h"

namespace opentxs
{
auto translate(const blind::CashType in) noexcept -> proto::CashType;
auto translate(const blind::PurseType in) noexcept -> proto::PurseType;
auto translate(const blind::TokenState in) noexcept -> proto::TokenState;
auto translate(const proto::CashType in) noexcept -> blind::CashType;
auto translate(const proto::PurseType in) noexcept -> blind::PurseType;
auto translate(const proto::TokenState in) noexcept -> blind::TokenState;
}  // namespace opentxs

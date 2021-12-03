// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/contact/Attribute.hpp"
// IWYU pragma: no_include "opentxs/contact/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/contact/SectionType.hpp"

#pragma once

#include "opentxs/contact/Types.hpp"
#include "serialization/protobuf/ContactEnums.pb.h"

namespace opentxs
{
auto translate(const contact::Attribute in) noexcept
    -> proto::ContactItemAttribute;
auto translate(const contact::ClaimType in) noexcept -> proto::ContactItemType;
auto translate(const contact::SectionType in) noexcept
    -> proto::ContactSectionName;
auto translate(const proto::ContactItemAttribute in) noexcept
    -> contact::Attribute;
auto translate(const proto::ContactItemType in) noexcept -> contact::ClaimType;
auto translate(const proto::ContactSectionName in) noexcept
    -> contact::SectionType;
}  // namespace opentxs

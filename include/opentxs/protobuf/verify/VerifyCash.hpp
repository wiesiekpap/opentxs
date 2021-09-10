// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFYCASH_HPP
#define OPENTXS_PROTOBUF_VERIFYCASH_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"

namespace opentxs
{
namespace proto
{
auto LucreTokenDataAllowedCiphertext() noexcept -> const VersionMap&;
auto PurseAllowedCiphertext() noexcept -> const VersionMap&;
auto PurseAllowedEnvelope() noexcept -> const VersionMap&;
auto PurseAllowedSymmetricKey() noexcept -> const VersionMap&;
auto PurseAllowedToken() noexcept -> const VersionMap&;
auto PurseExchangeAllowedPurse() noexcept -> const VersionMap&;
auto TokenAllowedLucreTokenData() noexcept -> const VersionMap&;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYCASH_HPP

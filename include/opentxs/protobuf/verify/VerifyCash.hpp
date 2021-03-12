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
OPENTXS_EXPORT const VersionMap& LucreTokenDataAllowedCiphertext() noexcept;
OPENTXS_EXPORT const VersionMap& PurseAllowedCiphertext() noexcept;
OPENTXS_EXPORT const VersionMap& PurseAllowedEnvelope() noexcept;
OPENTXS_EXPORT const VersionMap& PurseAllowedSymmetricKey() noexcept;
OPENTXS_EXPORT const VersionMap& PurseAllowedToken() noexcept;
OPENTXS_EXPORT const VersionMap& PurseExchangeAllowedPurse() noexcept;
OPENTXS_EXPORT const VersionMap& TokenAllowedLucreTokenData() noexcept;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYCASH_HPP

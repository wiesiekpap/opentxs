// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFYCONTRACTS_HPP
#define OPENTXS_PROTOBUF_VERIFYCONTRACTS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT const VersionMap& BasketParamsAllowedBasketItem() noexcept;
OPENTXS_EXPORT const VersionMap& IssuerAllowedPeerRequestHistory() noexcept;
OPENTXS_EXPORT const VersionMap& IssuerAllowedUnitAccountMap() noexcept;
OPENTXS_EXPORT const VersionMap&
PeerRequestHistoryAllowedPeerRequestWorkflow() noexcept;
OPENTXS_EXPORT const VersionMap& ServerContractAllowedListenAddress() noexcept;
OPENTXS_EXPORT const VersionMap& ServerContractAllowedNym() noexcept;
OPENTXS_EXPORT const VersionMap& ServerContractAllowedSignature() noexcept;
OPENTXS_EXPORT const VersionMap& UnitDefinitionAllowedBasketParams() noexcept;
OPENTXS_EXPORT const VersionMap& UnitDefinitionAllowedCurrencyParams() noexcept;
OPENTXS_EXPORT const VersionMap& UnitDefinitionAllowedNym() noexcept;
OPENTXS_EXPORT const VersionMap& UnitDefinitionAllowedSecurityParams() noexcept;
OPENTXS_EXPORT const VersionMap& UnitDefinitionAllowedSignature() noexcept;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYCONTRACTS_HPP

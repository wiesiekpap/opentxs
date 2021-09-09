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
auto BasketParamsAllowedBasketItem() noexcept -> const VersionMap&;
auto IssuerAllowedPeerRequestHistory() noexcept -> const VersionMap&;
auto IssuerAllowedUnitAccountMap() noexcept -> const VersionMap&;
auto PeerRequestHistoryAllowedPeerRequestWorkflow() noexcept
    -> const VersionMap&;
auto ServerContractAllowedListenAddress() noexcept -> const VersionMap&;
auto ServerContractAllowedNym() noexcept -> const VersionMap&;
auto ServerContractAllowedSignature() noexcept -> const VersionMap&;
auto UnitDefinitionAllowedBasketParams() noexcept -> const VersionMap&;
auto UnitDefinitionAllowedCurrencyParams() noexcept -> const VersionMap&;
auto UnitDefinitionAllowedNym() noexcept -> const VersionMap&;
auto UnitDefinitionAllowedSecurityParams() noexcept -> const VersionMap&;
auto UnitDefinitionAllowedSignature() noexcept -> const VersionMap&;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYCONTRACTS_HPP

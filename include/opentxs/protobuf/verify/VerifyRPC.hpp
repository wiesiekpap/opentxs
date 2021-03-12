// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFYRPC_HPP
#define OPENTXS_PROTOBUF_VERIFYRPC_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT const VersionMap& AddClaimAllowedContactItem() noexcept;
OPENTXS_EXPORT const VersionMap& ContactEventAllowedAccountEvent() noexcept;
OPENTXS_EXPORT const VersionMap& CreateNymAllowedAddClaim() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedAPIArgument() noexcept;
OPENTXS_EXPORT const VersionMap&
RPCCommandAllowedAcceptPendingPayment() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedAddClaim() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedAddContact() noexcept;
OPENTXS_EXPORT const VersionMap&
RPCCommandAllowedCreateInstrumentDefinition() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedCreateNym() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedGetWorkflow() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedHDSeed() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedModifyAccount() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedSendMessage() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedSendPayment() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedServerContract() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedVerification() noexcept;
OPENTXS_EXPORT const VersionMap& RPCCommandAllowedVerifyClaim() noexcept;
OPENTXS_EXPORT const VersionMap& RPCPushAllowedAccountEvent() noexcept;
OPENTXS_EXPORT const VersionMap& RPCPushAllowedContactEvent() noexcept;
OPENTXS_EXPORT const VersionMap& RPCPushAllowedTaskComplete() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedAccountData() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedAccountEvent() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedContact() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedContactEvent() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedHDSeed() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedNym() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedRPCStatus() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedRPCTask() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedServerContract() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedSessionData() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedTransactionData() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedUnitDefinition() noexcept;
OPENTXS_EXPORT const VersionMap& RPCResponseAllowedWorkflow() noexcept;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYRPC_HPP

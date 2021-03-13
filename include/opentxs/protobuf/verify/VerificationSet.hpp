// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFICATIONSET_HPP
#define OPENTXS_PROTOBUF_VERIFICATIONSET_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/verify/VerifyContacts.hpp"

namespace opentxs
{
namespace proto
{
class VerificationSet;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT bool CheckProto_1(
    const VerificationSet& verificationSet,
    const bool silent,
    const VerificationType indexed);
OPENTXS_EXPORT bool CheckProto_2(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_3(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_4(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_5(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_6(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_7(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_8(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_9(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_10(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_11(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_12(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_13(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_14(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_15(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_16(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_17(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_18(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_19(
    const VerificationSet&,
    const bool,
    const VerificationType);
OPENTXS_EXPORT bool CheckProto_20(
    const VerificationSet&,
    const bool,
    const VerificationType);
}  // namespace proto
}  // namespace opentxs

#endif  // OPENTXS_PROTOBUF_VERIFICATIONSET_HPP

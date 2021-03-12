// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_SOURCEPROOF_HPP
#define OPENTXS_PROTOBUF_SOURCEPROOF_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace proto
{
class SourceProof;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT bool CheckProto_1(
    const SourceProof& proot,
    const bool silent,
    bool& ExpectSourceSignature);
OPENTXS_EXPORT bool CheckProto_2(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_3(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_4(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_5(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_6(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_7(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_8(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_9(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_10(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_11(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_12(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_13(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_14(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_15(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_16(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_17(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_18(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_19(const SourceProof&, const bool, bool&);
OPENTXS_EXPORT bool CheckProto_20(const SourceProof&, const bool, bool&);
}  // namespace proto
}  // namespace opentxs

#endif  // OPENTXS_PROTOBUF_SOURCEPROOF_HPP

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_CIPHERTEXT_HPP
#define OPENTXS_PROTOBUF_CIPHERTEXT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace proto
{
class Ciphertext;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT bool CheckProto_1(
    const Ciphertext& data,
    const bool silent,
    const bool nested);
OPENTXS_EXPORT bool CheckProto_2(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_3(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_4(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_5(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_6(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_7(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_8(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_9(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_10(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_11(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_12(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_13(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_14(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_15(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_16(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_17(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_18(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_19(const Ciphertext&, const bool, const bool);
OPENTXS_EXPORT bool CheckProto_20(const Ciphertext&, const bool, const bool);
}  // namespace proto
}  // namespace opentxs

#endif  // OPENTXS_PROTOBUF_CIPHERTEXT_HPP

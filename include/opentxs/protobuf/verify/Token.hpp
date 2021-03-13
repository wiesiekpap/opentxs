// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_TOKEN_HPP
#define OPENTXS_PROTOBUF_TOKEN_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/CashEnums.pb.h"

namespace opentxs
{
namespace proto
{
class Token;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT bool CheckProto_1(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_2(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_3(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_4(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_5(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_6(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_7(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_8(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_9(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_10(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_11(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_12(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_13(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_14(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_15(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_16(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_17(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_18(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_19(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
OPENTXS_EXPORT bool CheckProto_20(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::set<TokenState>& expectedState,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo);
}  // namespace proto
}  // namespace opentxs

#endif  // OPENTXS_PROTOBUF_TOKEN_HPP

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/BlockchainDeterministicAccountData.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/BlockchainDeterministicAccountData.pb.h"
#include "opentxs/protobuf/verify/BlockchainAccountData.hpp"
#include "opentxs/protobuf/verify/HDPath.hpp"
#include "opentxs/protobuf/verify/VerifyBlockchain.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "blockchain deterministic account data"

namespace opentxs::proto
{
auto CheckProto_1(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    CHECK_SUBOBJECT(
        common,
        BlockchainDeterministicAccountDataAllowedBlockchainAccountData());
    CHECK_SUBOBJECT(path, BlockchainDeterministicAccountDataAllowedHDPath());

    return true;
}

auto CheckProto_2(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const BlockchainDeterministicAccountData& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto

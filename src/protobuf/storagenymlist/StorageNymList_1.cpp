// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/StorageNymList.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/StorageNymList.pb.h"
#include "opentxs/protobuf/verify/StorageBip47NymAddressIndex.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/StorageItemHash.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/VerifyStorage.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "nym index"

namespace opentxs
{
namespace proto
{

auto CheckProto_1(const StorageNymList& input, const bool silent) -> bool
{
    CHECK_SUBOBJECTS(nym, StorageNymListAllowedStorageItemHash());
    CHECK_NONE(localnymid)
    CHECK_NONE(address)
    CHECK_NONE(transaction)

    return true;
}

auto CheckProto_2(const StorageNymList& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const StorageNymList& input, const bool silent) -> bool
{
    CHECK_SUBOBJECTS(nym, StorageNymListAllowedStorageItemHash());
    CHECK_IDENTIFIERS(localnymid)
    CHECK_NONE(address)
    CHECK_NONE(transaction)
    return true;
}

auto CheckProto_4(const StorageNymList& input, const bool silent) -> bool
{
    CHECK_SUBOBJECTS(nym, StorageNymListAllowedStorageItemHash());
    CHECK_IDENTIFIERS(localnymid)
    OPTIONAL_SUBOBJECTS(
        address, StorageNymListAllowedStorageBip47NymAddressIndex())
    OPTIONAL_SUBOBJECTS(
        transaction, StorageNymListAllowedStorageBip47NymAddressIndex())

    return true;
}

auto CheckProto_5(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const StorageNymList& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs

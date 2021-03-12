// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/StorageContacts.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/StorageContactAddressIndex.pb.h"
#include "opentxs/protobuf/StorageContactNymIndex.pb.h"
#include "opentxs/protobuf/StorageContacts.pb.h"
#include "opentxs/protobuf/StorageIDList.pb.h"
#include "opentxs/protobuf/StorageItemHash.pb.h"
#include "opentxs/protobuf/verify/StorageContactAddressIndex.hpp"
#include "opentxs/protobuf/verify/StorageContactNymIndex.hpp"
#include "opentxs/protobuf/verify/StorageIDList.hpp"
#include "opentxs/protobuf/verify/StorageItemHash.hpp"
#include "opentxs/protobuf/verify/VerifyStorage.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "contact storage index"

namespace opentxs::proto
{
auto CheckProto_2(const StorageContacts& input, const bool silent) -> bool
{
    for (auto& merge : input.merge()) {
        try {
            const bool valid = Check(
                merge,
                StorageContactsAllowedList().at(input.version()).first,
                StorageContactsAllowedList().at(input.version()).second,
                silent);

            if (!valid) { FAIL_1("invalid merge") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed storage id list version not defined for version",
                input.version())
        }
    }

    for (auto& hash : input.contact()) {
        try {
            const bool valid = Check(
                hash,
                StorageContactsAllowedStorageItemHash()
                    .at(input.version())
                    .first,
                StorageContactsAllowedStorageItemHash()
                    .at(input.version())
                    .second,
                silent);

            if (!valid) { FAIL_1("invalid hash") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed storage item hash version not defined for version",
                input.version())
        }
    }

    for (auto& index : input.address()) {
        try {
            const bool valid = Check(
                index,
                StorageContactsAllowedAddress().at(input.version()).first,
                StorageContactsAllowedAddress().at(input.version()).second,
                silent);

            if (!valid) { FAIL_1("invalid address index") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed address index version not defined for version",
                input.version())
        }
    }

    for (auto& index : input.nym()) {
        try {
            const bool valid = Check(
                index,
                StorageContactsAllowedStorageContactNymIndex()
                    .at(input.version())
                    .first,
                StorageContactsAllowedStorageContactNymIndex()
                    .at(input.version())
                    .second,
                silent);

            if (!valid) { FAIL_1("invalid nym index") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed nym index version not defined for version",
                input.version())
        }
    }

    return true;
}

auto CheckProto_3(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const StorageContacts& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto

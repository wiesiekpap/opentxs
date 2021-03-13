// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_STORAGECREDENTIALS_HPP
#define OPENTXS_PROTOBUF_STORAGECREDENTIALS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace proto
{
class StorageCredentials;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT bool CheckProto_1(
    const StorageCredentials& creds,
    const bool silent);
OPENTXS_EXPORT bool CheckProto_2(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_3(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_4(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_5(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_6(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_7(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_8(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_9(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_10(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_11(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_12(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_13(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_14(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_15(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_16(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_17(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_18(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_19(const StorageCredentials&, const bool);
OPENTXS_EXPORT bool CheckProto_20(const StorageCredentials&, const bool);
}  // namespace proto
}  // namespace opentxs

#endif  // OPENTXS_PROTOBUF_STORAGECREDENTIALS_HPP

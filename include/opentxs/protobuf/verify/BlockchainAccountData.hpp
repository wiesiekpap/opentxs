// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_BLOCKCHAIN_ACCOUNT_DATA_HPP
#define OPENTXS_PROTOBUF_BLOCKCHAIN_ACCOUNT_DATA_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace proto
{
class BlockchainAccountData;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT bool CheckProto_1(
    const BlockchainAccountData& account,
    const bool silent);
OPENTXS_EXPORT bool CheckProto_2(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_3(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_4(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_5(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_6(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_7(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_8(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_9(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_10(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_11(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_12(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_13(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_14(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_15(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_16(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_17(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_18(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_19(const BlockchainAccountData&, const bool);
OPENTXS_EXPORT bool CheckProto_20(const BlockchainAccountData&, const bool);
}  // namespace proto
}  // namespace opentxs

#endif  // OPENTXS_PROTOBUF_BLOCKCHAIN_ACCOUNT_DATA_HPP

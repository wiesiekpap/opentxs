// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_BLOCKCHAIN_TRANSACTION_HPP
#define OPENTXS_PROTOBUF_BLOCKCHAIN_TRANSACTION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace proto
{
class BlockchainTransaction;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
auto CheckProto_1(const BlockchainTransaction& transaction, const bool silent)
    -> bool;
auto CheckProto_2(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_3(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_4(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_5(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_6(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_7(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_8(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_9(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_10(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_11(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_12(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_13(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_14(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_15(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_16(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_17(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_18(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_19(const BlockchainTransaction&, const bool) -> bool;
auto CheckProto_20(const BlockchainTransaction&, const bool) -> bool;
}  // namespace proto
}  // namespace opentxs

#endif  // OPENTXS_PROTOBUF_BLOCKCHAIN_TRANSACTION_HPP

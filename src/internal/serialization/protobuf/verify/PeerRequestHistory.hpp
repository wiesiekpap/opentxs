// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class PeerRequestHistory;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::proto
{
auto CheckProto_1(const PeerRequestHistory& input, const bool silent) -> bool;
auto CheckProto_2(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_3(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_4(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_5(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_6(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_7(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_8(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_9(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_10(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_11(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_12(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_13(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_14(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_15(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_16(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_17(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_18(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_19(const PeerRequestHistory&, const bool) -> bool;
auto CheckProto_20(const PeerRequestHistory&, const bool) -> bool;
}  // namespace opentxs::proto

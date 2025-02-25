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
class PendingBailment;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::proto
{
auto CheckProto_1(const PendingBailment& request, const bool silent) -> bool;
auto CheckProto_2(const PendingBailment& request, const bool silent) -> bool;
auto CheckProto_3(const PendingBailment& request, const bool silent) -> bool;
auto CheckProto_4(const PendingBailment& request, const bool silent) -> bool;
auto CheckProto_5(const PendingBailment& request, const bool silent) -> bool;
auto CheckProto_6(const PendingBailment& request, const bool) -> bool;
auto CheckProto_7(const PendingBailment& request, const bool) -> bool;
auto CheckProto_8(const PendingBailment& request, const bool) -> bool;
auto CheckProto_9(const PendingBailment& request, const bool) -> bool;
auto CheckProto_10(const PendingBailment& request, const bool) -> bool;
auto CheckProto_11(const PendingBailment& request, const bool) -> bool;
auto CheckProto_12(const PendingBailment& request, const bool) -> bool;
auto CheckProto_13(const PendingBailment& request, const bool) -> bool;
auto CheckProto_14(const PendingBailment& request, const bool) -> bool;
auto CheckProto_15(const PendingBailment& request, const bool) -> bool;
auto CheckProto_16(const PendingBailment& request, const bool) -> bool;
auto CheckProto_17(const PendingBailment& request, const bool) -> bool;
auto CheckProto_18(const PendingBailment& request, const bool) -> bool;
auto CheckProto_19(const PendingBailment& request, const bool) -> bool;
auto CheckProto_20(const PendingBailment& request, const bool) -> bool;
}  // namespace opentxs::proto

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
struct NymListRow {
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
};

struct NymListData {
    ot::UnallocatedVector<NymListRow> rows_{};
};

auto check_nym_list(
    const ot::api::session::Client& api,
    const NymListData& expected) noexcept -> bool;
auto check_nym_list_qt(
    const ot::api::session::Client& api,
    const NymListData& expected) noexcept -> bool;
auto init_nym_list(
    const ot::api::session::Client& api,
    Counter& counter) noexcept -> void;
}  // namespace ottest

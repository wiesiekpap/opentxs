// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstddef>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
struct SeedTreeNym {
    std::size_t index_{};
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
};

struct SeedTreeItem {
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
    ot::crypto::SeedStyle type_{};
    ot::UnallocatedVector<SeedTreeNym> rows_;
};

struct SeedTreeData {
    ot::UnallocatedVector<SeedTreeItem> rows_;
};

auto check_seed_tree(
    const ot::api::session::Client& api,
    const SeedTreeData& expected) noexcept -> bool;
auto check_seed_tree_qt(
    const ot::api::session::Client& api,
    const SeedTreeData& expected) noexcept -> bool;
auto init_seed_tree(
    const ot::api::session::Client& api,
    Counter& counter) noexcept -> void;
auto print_seed_tree(const ot::api::session::Client& api) noexcept
    -> ot::UnallocatedCString;
}  // namespace ottest

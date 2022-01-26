// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "opentxs/util/Container.hpp"  // IWYU pragma: associated

namespace opentxs::alloc
{
// TODO not yet supported on Android
// auto System() noexcept -> Resource*
// {
// #if __has_include(<memory_resource>)
//     return std::pmr::new_delete_resource();
// #else
//     return std::experimental::pmr::new_delete_resource();
// #endif
// }
//
// auto Null() noexcept -> Resource*
// {
// #if __has_include(<memory_resource>)
//     return std::pmr::null_memory_resource();
// #else
//     return std::experimental::pmr::null_memory_resource();
// #endif
// }
}  // namespace opentxs::alloc

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated

#include "2_Factory.hpp"

namespace opentxs
{
auto Factory::StorageFSArchive(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket,
    const std::string& folder,
    crypto::key::Symmetric& key) -> opentxs::api::storage::Plugin*
{
    return nullptr;
}

auto Factory::StorageFSGC(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket) -> opentxs::api::storage::Plugin*
{
    return nullptr;
}
}  // namespace opentxs

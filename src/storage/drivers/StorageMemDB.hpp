// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <map>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Lockable.hpp"
#include "storage/Plugin.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Plugin;
class Storage;
}  // namespace storage
}  // namespace api

class Factory;
class Flag;
class StorageConfig;
}  // namespace opentxs

namespace opentxs::storage::implementation
{
// In-memory implementation of opentxs::storage
class StorageMemDB final : public Plugin,
                           virtual public opentxs::api::storage::Driver,
                           Lockable
{
public:
    auto EmptyBucket(const bool bucket) const -> bool final;
    auto LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const -> bool final;
    auto LoadRoot() const -> std::string final;
    auto StoreRoot(const bool commit, const std::string& hash) const
        -> bool final;

    void Cleanup() final {}

    ~StorageMemDB() final = default;

private:
    using ot_super = Plugin;

    friend opentxs::Factory;

    mutable std::string root_{""};
    mutable std::map<std::string, std::string> a_{};
    mutable std::map<std::string, std::string> b_{};

    void store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>* promise) const final;

    StorageMemDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
    StorageMemDB() = delete;
    StorageMemDB(const StorageMemDB&) = delete;
    StorageMemDB(StorageMemDB&&) = delete;
    auto operator=(const StorageMemDB&) -> StorageMemDB& = delete;
    auto operator=(StorageMemDB&&) -> StorageMemDB& = delete;
};
}  // namespace opentxs::storage::implementation

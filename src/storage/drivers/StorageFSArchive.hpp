// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "storage/drivers/StorageFS.hpp"

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

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto

class Factory;
class Flag;
class StorageConfig;
}  // namespace opentxs

namespace opentxs::storage::implementation
{
class StorageFSArchive final : public StorageFS,
                               public virtual opentxs::api::storage::Driver
{
private:
    using ot_super = StorageFS;

public:
    auto EmptyBucket(const bool bucket) const -> bool final;

    void Cleanup() final;

    ~StorageFSArchive() final;

private:
    friend Factory;

    crypto::key::Symmetric& encryption_key_;
    const bool encrypted_{false};

    auto calculate_path(
        const std::string& key,
        const bool bucket,
        std::string& directory) const -> std::string final;
    auto prepare_read(const std::string& ciphertext) const -> std::string final;
    auto prepare_write(const std::string& plaintext) const -> std::string final;
    auto root_filename() const -> std::string final;

    void Init_StorageFSArchive();
    void Cleanup_StorageFSArchive();

    StorageFSArchive(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket,
        const std::string& folder,
        crypto::key::Symmetric& key);
    StorageFSArchive() = delete;
    StorageFSArchive(const StorageFSArchive&) = delete;
    StorageFSArchive(StorageFSArchive&&) = delete;
    auto operator=(const StorageFSArchive&) -> StorageFSArchive& = delete;
    auto operator=(StorageFSArchive&&) -> StorageFSArchive& = delete;
};
}  // namespace opentxs::storage::implementation

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "storage/drivers/StorageFSArchive.hpp"  // IWYU pragma: associated

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <memory>

#include "2_Factory.hpp"
#include "Proto.tpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/protobuf/Ciphertext.pb.h"
#include "storage/StorageConfig.hpp"

#define ROOT_FILE_EXTENSION ".hash"

#define OT_METHOD "opentxs::StorageFSArchive::"

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
    return new opentxs::storage::implementation::StorageFSArchive(
        storage, config, hash, random, bucket, folder, key);
}
}  // namespace opentxs

namespace opentxs::storage::implementation
{
StorageFSArchive::StorageFSArchive(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket,
    const std::string& folder,
    crypto::key::Symmetric& key)
    : ot_super(storage, config, hash, random, folder, bucket)
    , encryption_key_(key)
    , encrypted_(bool(encryption_key_))
{
    Init_StorageFSArchive();
}

auto StorageFSArchive::calculate_path(
    const std::string& key,
    const bool,
    std::string& directory) const -> std::string
{
    directory = folder_;
    auto& level1 = folder_;
    std::string level2{};

    if (4 < key.size()) {
        directory += path_seperator_;
        directory += key.substr(0, 4);
        level2 = directory;
    }

    if (8 < key.size()) {
        directory += path_seperator_;
        directory += key.substr(4, 4);
    }

    boost::system::error_code ec{};
    boost::filesystem::create_directories(directory, ec);

    if (8 < key.size()) {
        if (false == sync(level2)) {
            LogOutput(OT_METHOD)(__func__)(": Unable to sync directory ")(
                level2)(".")
                .Flush();
        }
    }

    if (false == sync(level1)) {
        LogOutput(OT_METHOD)(__func__)(": Unable to sync directory ")(
            level1)(".")
            .Flush();
    }

    return {directory + path_seperator_ + key};
}

void StorageFSArchive::Cleanup()
{
    Cleanup_StorageFSArchive();
    ot_super::Cleanup();
}

void StorageFSArchive::Cleanup_StorageFSArchive()
{
    // future cleanup actions go here
}

auto StorageFSArchive::EmptyBucket(const bool) const -> bool { return true; }

void StorageFSArchive::Init_StorageFSArchive()
{
    OT_ASSERT(false == folder_.empty());

    boost::system::error_code ec{};

    if (boost::filesystem::create_directory(folder_, ec)) { ready_->On(); }
}

auto StorageFSArchive::prepare_read(const std::string& input) const
    -> std::string
{
    if (false == encrypted_) { return input; }

    const auto ciphertext = proto::Factory<proto::Ciphertext>(input);

    OT_ASSERT(encryption_key_);

    std::string output{};
    auto reason =
        encryption_key_.api().Factory().PasswordPrompt("Storage read");

    if (false == encryption_key_.Decrypt(ciphertext, reason, writer(output))) {
        LogOutput(OT_METHOD)(__func__)(": Failed to decrypt value.").Flush();
    }

    return output;
}

auto StorageFSArchive::prepare_write(const std::string& plaintext) const
    -> std::string
{
    if (false == encrypted_) { return plaintext; }

    OT_ASSERT(encryption_key_);

    proto::Ciphertext ciphertext{};
    auto reason =
        encryption_key_.api().Factory().PasswordPrompt("Storage write");
    const bool encrypted =
        encryption_key_.Encrypt(plaintext, reason, ciphertext, false);

    if (false == encrypted) {
        LogOutput(OT_METHOD)(__func__)(": Failed to encrypt value.").Flush();
    }

    return proto::ToString(ciphertext);
}

auto StorageFSArchive::root_filename() const -> std::string
{
    return folder_ + path_seperator_ + config_.fs_root_file_ +
           ROOT_FILE_EXTENSION;
}

StorageFSArchive::~StorageFSArchive() { Cleanup_StorageFSArchive(); }
}  // namespace opentxs::storage::implementation

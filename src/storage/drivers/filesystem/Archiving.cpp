// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "storage/drivers/filesystem/Archiving.hpp"  // IWYU pragma: associated

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <memory>

#include "Proto.tpp"
#include "internal/storage/drivers/Factory.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/protobuf/Ciphertext.pb.h"
#include "storage/Config.hpp"

#define ROOT_FILE_EXTENSION ".hash"

#define OT_METHOD "opentxs::storage::driver::filesystem::Archiving::"

namespace opentxs::factory
{
auto StorageFSArchive(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::storage::Storage& parent,
    const storage::Config& config,
    const Flag& bucket,
    const std::string& folder,
    crypto::key::Symmetric& key) noexcept -> std::unique_ptr<storage::Plugin>
{
    using ReturnType = storage::driver::filesystem::Archiving;

    return std::make_unique<ReturnType>(
        crypto, asio, parent, config, bucket, folder, key);
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver::filesystem
{
Archiving::Archiving(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::storage::Storage& storage,
    const storage::Config& config,
    const Flag& bucket,
    const std::string& folder,
    crypto::key::Symmetric& key)
    : ot_super(crypto, asio, storage, config, folder, bucket)
    , encryption_key_(key)
    , encrypted_(bool(encryption_key_))
{
    Init_Archiving();
}

auto Archiving::calculate_path(
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

void Archiving::Cleanup()
{
    Cleanup_Archiving();
    ot_super::Cleanup();
}

void Archiving::Cleanup_Archiving()
{
    // future cleanup actions go here
}

auto Archiving::EmptyBucket(const bool) const -> bool { return true; }

void Archiving::Init_Archiving()
{
    OT_ASSERT(false == folder_.empty());

    boost::system::error_code ec{};

    if (boost::filesystem::create_directory(folder_, ec)) { ready_->On(); }
}

auto Archiving::prepare_read(const std::string& input) const -> std::string
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

auto Archiving::prepare_write(const std::string& plaintext) const -> std::string
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

auto Archiving::root_filename() const -> std::string
{
    return folder_ + path_seperator_ + config_.fs_root_file_ +
           ROOT_FILE_EXTENSION;
}

Archiving::~Archiving() { Cleanup_Archiving(); }
}  // namespace opentxs::storage::driver::filesystem

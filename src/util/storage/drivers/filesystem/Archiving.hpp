// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/drivers/filesystem/Common.hpp"

namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network

namespace session
{
class Storage;
}  // namespace session

class Crypto;
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto

namespace storage
{
class Config;
class Plugin;
}  // namespace storage

class Flag;
}  // namespace opentxs

namespace opentxs::storage::driver::filesystem
{
class Archiving final : public Common, public virtual storage::Driver
{
private:
    using ot_super = Common;

public:
    auto EmptyBucket(const bool bucket) const -> bool final;

    void Cleanup() final;

    Archiving(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket,
        const std::string& folder,
        crypto::key::Symmetric& key);

    ~Archiving() final;

private:
    crypto::key::Symmetric& encryption_key_;
    const bool encrypted_;

    auto calculate_path(
        const std::string& key,
        const bool bucket,
        std::string& directory) const -> std::string final;
    auto prepare_read(const std::string& ciphertext) const -> std::string final;
    auto prepare_write(const std::string& plaintext) const -> std::string final;
    auto root_filename() const -> std::string final;

    void Init_Archiving();
    void Cleanup_Archiving();

    Archiving() = delete;
    Archiving(const Archiving&) = delete;
    Archiving(Archiving&&) = delete;
    auto operator=(const Archiving&) -> Archiving& = delete;
    auto operator=(Archiving&&) -> Archiving& = delete;
};
}  // namespace opentxs::storage::driver::filesystem

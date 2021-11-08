// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/storage/Driver.hpp"
#include "opentxs/util/Bytes.hpp"
#include "storage/drivers/filesystem/Common.hpp"

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

namespace storage
{
class Config;
class Plugin;
}  // namespace storage

class Flag;
}  // namespace opentxs

namespace opentxs::storage::driver::filesystem
{
// Simple filesystem implementation of opentxs::storage
class GarbageCollected final : public Common, public virtual storage::Driver
{
private:
    using ot_super = Common;

public:
    auto EmptyBucket(const bool bucket) const -> bool final;

    void Cleanup() final;

    GarbageCollected(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket);

    ~GarbageCollected() final;

private:
    auto bucket_name(const bool bucket) const -> std::string;
    auto calculate_path(
        const std::string& key,
        const bool bucket,
        std::string& directory) const -> std::string final;
    void purge(const std::string& path) const;
    auto root_filename() const -> std::string final;

    void Cleanup_GarbageCollected();
    void Init_GarbageCollected();

    GarbageCollected() = delete;
    GarbageCollected(const GarbageCollected&) = delete;
    GarbageCollected(GarbageCollected&&) = delete;
    auto operator=(const GarbageCollected&) -> GarbageCollected& = delete;
    auto operator=(GarbageCollected&&) -> GarbageCollected& = delete;
};
}  // namespace opentxs::storage::driver::filesystem

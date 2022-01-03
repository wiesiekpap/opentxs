// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <atomic>
#include <future>
#include <ios>
#include <string>

#include "internal/util/Flag.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/util/Bytes.hpp"
#include "util/storage/Plugin.hpp"

namespace boost
{
namespace iostreams
{
class file_descriptor_sink;
}  // namespace iostreams
}  // namespace boost

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
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage::driver::filesystem
{
// Simple filesystem implementation of opentxs::storage
class Common : public implementation::Plugin
{
private:
    using ot_super = Plugin;

public:
    auto LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const -> bool override;
    auto LoadRoot() const -> std::string override;
    auto StoreRoot(const bool commit, const std::string& hash) const
        -> bool override;

    void Cleanup() override;

    ~Common() override;

protected:
    const std::string folder_;
    const std::string path_seperator_;
    OTFlag ready_;

    auto sync(const std::string& path) const -> bool;

    Common(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const std::string& folder,
        const Flag& bucket);

private:
    using File =
        boost::iostreams::stream<boost::iostreams::file_descriptor_sink>;

    virtual auto calculate_path(
        const std::string& key,
        const bool bucket,
        std::string& directory) const -> std::string = 0;
    virtual auto prepare_read(const std::string& input) const -> std::string;
    virtual auto prepare_write(const std::string& input) const -> std::string;
    auto read_file(const std::string& filename) const -> std::string;
    virtual auto root_filename() const -> std::string = 0;
    void store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>* promise) const override;
    auto sync(File& file) const -> bool;
    auto sync(int fd) const -> bool;
    auto write_file(
        const std::string& directory,
        const std::string& filename,
        const std::string& contents) const -> bool;

    void Cleanup_Common();
    void Init_Common();

    Common() = delete;
    Common(const Common&) = delete;
    Common(Common&&) = delete;
    auto operator=(const Common&) -> Common& = delete;
    auto operator=(Common&&) -> Common& = delete;
};
}  // namespace opentxs::storage::driver::filesystem

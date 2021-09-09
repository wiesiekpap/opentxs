// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/iostreams/detail/wrap_unwrap.hpp>

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "storage/drivers/StorageFS.hpp"  // IWYU pragma: associated

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/system/error_code.hpp>
#include <fstream>
#include <ios>
#include <memory>
#include <vector>

#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define PATH_SEPERATOR "/"

#define OT_METHOD "opentxs::StorageFS::"

namespace opentxs
{
StorageFS::StorageFS(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const std::string& folder,
    const Flag& bucket)
    : ot_super(storage, config, hash, random, bucket)
    , folder_(folder)
    , path_seperator_(PATH_SEPERATOR)
    , ready_(Flag::Factory(false))
{
    Init_StorageFS();
}

void StorageFS::Cleanup() { Cleanup_StorageFS(); }

void StorageFS::Cleanup_StorageFS()
{
    // future cleanup actions go here
}

void StorageFS::Init_StorageFS()
{
    // future init actions go here
}

auto StorageFS::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const -> bool
{
    value.clear();
    std::string directory{};
    const auto filename = calculate_path(key, bucket, directory);
    boost::system::error_code ec{};

    if (false == boost::filesystem::exists(filename, ec)) { return false; }

    if (ready_.get() && false == folder_.empty()) {
        value = read_file(filename);
    }

    return false == value.empty();
}

auto StorageFS::LoadRoot() const -> std::string
{
    if (ready_.get() && false == folder_.empty()) {

        return read_file(root_filename());
    }

    return "";
}

auto StorageFS::prepare_read(const std::string& input) const -> std::string
{
    return input;
}

auto StorageFS::prepare_write(const std::string& input) const -> std::string
{
    return input;
}

auto StorageFS::read_file(const std::string& filename) const -> std::string
{
    boost::system::error_code ec{};

    if (false == boost::filesystem::exists(filename, ec)) { return {}; }

    std::ifstream file(
        filename, std::ios::in | std::ios::ate | std::ios::binary);

    if (file.good()) {
        std::ifstream::pos_type pos = file.tellg();

        if ((0 >= pos) || (0xFFFFFFFF <= pos)) { return {}; }

        auto size(pos);
        file.seekg(0, std::ios::beg);
        std::vector<char> bytes(size);
        file.read(&bytes[0], size);

        return prepare_read(std::string(&bytes[0], size));
    }

    return {};
}

void StorageFS::store(
    const bool,
    const std::string& key,
    const std::string& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    if (ready_.get() && false == folder_.empty()) {
        std::string directory{};
        const auto filename = calculate_path(key, bucket, directory);
        promise->set_value(write_file(directory, filename, value));
    } else {
        promise->set_value(false);
    }
}

auto StorageFS::StoreRoot(const bool, const std::string& hash) const -> bool
{
    if (ready_.get() && false == folder_.empty()) {

        return write_file(folder_, root_filename(), hash);
    }

    return false;
}

auto StorageFS::sync(const std::string& path) const -> bool
{
    class FileDescriptor
    {
    public:
        FileDescriptor(const std::string& path)
            : fd_(::open(path.c_str(), O_DIRECTORY | O_RDONLY))
        {
        }

        operator bool() const { return good(); }
        operator int() const { return fd_; }

        ~FileDescriptor()
        {
            if (good()) { ::close(fd_); }
        }

    private:
        int fd_{-1};

        auto good() const -> bool { return (-1 != fd_); }

        FileDescriptor() = delete;
        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor(FileDescriptor&&) = delete;
        auto operator=(const FileDescriptor&) -> FileDescriptor& = delete;
        auto operator=(FileDescriptor&&) -> FileDescriptor& = delete;
    };

    FileDescriptor fd(path);

    if (!fd) {
        LogOutput(OT_METHOD)(__func__)(": Failed to open ")(path)(".").Flush();

        return false;
    }

    return sync(fd);
}

auto StorageFS::sync(File& file) const -> bool { return sync(file->handle()); }

auto StorageFS::sync(int fd) const -> bool
{
#if defined(__APPLE__)
    // This is a Mac OS X system which does not implement
    // fsync as such.
    return 0 == ::fcntl(fd, F_FULLFSYNC);
#else
    return 0 == ::fsync(fd);
#endif
}

auto StorageFS::write_file(
    const std::string& directory,
    const std::string& filename,
    const std::string& contents) const -> bool
{
    if (false == filename.empty()) {
        boost::filesystem::path filePath(filename);
        File file(filePath);
        const auto data = prepare_write(contents);

        if (file.good()) {
            file.write(data.c_str(), data.size());

            if (false == sync(file)) {
                LogOutput(OT_METHOD)(__func__)(": Failed to sync file ")(
                    filename)(".")
                    .Flush();
            }

            if (false == sync(directory)) {
                LogOutput(OT_METHOD)(__func__)(": Failed to sync directory ")(
                    directory)(".")
                    .Flush();
            }

            file.close();

            return true;
        } else {
            LogOutput(OT_METHOD)(__func__)(": Failed to write file.").Flush();
        }
    } else {
        LogOutput(OT_METHOD)(__func__)(": Failed to write empty filename.")
            .Flush();
    }

    return false;
}

StorageFS::~StorageFS() { Cleanup_StorageFS(); }

}  // namespace opentxs

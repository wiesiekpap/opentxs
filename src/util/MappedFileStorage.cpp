// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "util/MappedFileStorage.hpp"  // IWYU pragma: associated

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <algorithm>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "util/ByteLiterals.hpp"

#define OT_METHOD "opentxs::util::MappedFileStorage::"

namespace fs = boost::filesystem;

namespace opentxs::util
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

constexpr auto target_file_size_ =
#if OS_SUPPORTS_LARGE_SPARSE_FILES
    std::size_t{8_TiB};
#else
    std::size_t{1_GiB};
#endif  // OT_VALGRIND

constexpr auto get_file_count(const std::size_t bytes) noexcept -> std::size_t
{
    return std::max(
        std::size_t{1},
        ((bytes + 1u) / target_file_size_) +
            std::min(std::size_t{1}, (bytes + 1u) % target_file_size_));
}

using Offset = std::pair<std::size_t, std::size_t>;

constexpr auto get_offset(const std::size_t in) noexcept -> Offset
{
    return Offset{in / target_file_size_, in % target_file_size_};
}

constexpr auto get_start_position(const std::size_t file) noexcept
    -> std::size_t
{
    return file * target_file_size_;
}

struct MappedFileStorage::Imp {
    using FileCounter = std::size_t;

    LMDB& lmdb_;
    const std::string path_prefix_;
    const std::string filename_prefix_;
    const int table_;
    const std::size_t key_;
    mutable IndexData::MemoryPosition next_position_;
    mutable std::vector<boost::iostreams::mapped_file> files_;

    auto calculate_file_name(
        const std::string& prefix,
        const FileCounter index) noexcept -> std::string
    {
        auto number = std::to_string(index);

        while (5 > number.size()) { number.insert(0, 1, '0'); }

        const auto filename = filename_prefix_ + number + ".dat";
        auto path = fs::path{prefix};
        path /= filename;

        return path.string();
    }
    auto check_file(const FileCounter position) noexcept -> void
    {
        while (files_.size() < (position + 1)) {
            create_or_load(path_prefix_, files_.size(), files_);
        }
    }
    auto create_or_load(
        const std::string& prefix,
        const FileCounter file,
        std::vector<boost::iostreams::mapped_file>& output) noexcept -> void
    {
        auto params = boost::iostreams::mapped_file_params{
            calculate_file_name(prefix, file)};
        params.flags = boost::iostreams::mapped_file::readwrite;
        const auto& path = params.path;
        LogTrace(OT_METHOD)(__func__)(": initializing file ")(path).Flush();

        try {
            if (fs::exists(path)) {
                if (target_file_size_ == fs::file_size(path)) {
                    params.new_file_size = 0;
                } else {
                    LogOutput(OT_METHOD)(__func__)(": Incorrect size for ")(
                        path)
                        .Flush();
                    fs::remove(path);
                    params.new_file_size = target_file_size_;
                }
            } else {
                params.new_file_size = target_file_size_;
            }
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            OT_FAIL;
        }

        LogInsane(OT_METHOD)(__func__)(": new_file_size: ")(
            params.new_file_size)
            .Flush();

        try {
            output.emplace_back(params);
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            OT_FAIL;
        }
    }
    auto get_read_view(const IndexData& index) noexcept -> ReadView
    {
        const auto [file, offset] = get_offset(index.position_);
        check_file(file);

        return ReadView{files_.at(file).const_data() + offset, index.size_};
    }
    auto get_write_view(
        LMDB::Transaction& tx,
        IndexData& index,
        UpdateCallback&& cb,
        std::size_t bytes) noexcept -> WritableView
    {
        if (0 == bytes) { return {}; }

        const auto replace = bytes == index.size_;
        const auto output = [&] {
            const auto [file, offset] = get_offset(index.position_);
            check_file(file);

            return WritableView{files_.at(file).data() + offset, index.size_};
        };

        if (replace) {
            LogVerbose(OT_METHOD)(__func__)(": Replacing existing item")
                .Flush();

            return output();
        }

        increment_index(index, bytes);
        LogDebug(OT_METHOD)(__func__)(": Storing new item at position ")(
            index.position_)
            .Flush();
        const auto nextPosition = index.position_ + bytes;

        if (cb && (false == cb(tx))) { return {}; }

        if (false == update_next_position(nextPosition, tx)) {
            LogOutput(OT_METHOD)(__func__)(
                ": Failed to update next write position")
                .Flush();

            return {};
        }

        return output();
    }
    auto increment_index(IndexData& index, std::size_t bytes) noexcept -> void
    {
        index.size_ = bytes;
        index.position_ = next_position_;

        {
            // NOTE This check prevents writing past end of file
            const auto start = get_offset(index.position_).first;
            const auto end =
                get_offset(index.position_ + (index.size_ - 1)).first;

            if (end != start) {
                OT_ASSERT(end > start);

                index.position_ = get_start_position(end);
            }
        }
    }
    auto init_files(
        const std::string& prefix,
        const IndexData::MemoryPosition position) noexcept
        -> std::vector<boost::iostreams::mapped_file>
    {
        auto output = std::vector<boost::iostreams::mapped_file>{};
        const auto target = get_file_count(position);
        output.reserve(target);

        for (auto i = FileCounter{0}; i < target; ++i) {
            create_or_load(prefix, i, output);
        }

        return output;
    }
    auto load_position(opentxs::storage::lmdb::LMDB& db) noexcept
        -> IndexData::MemoryPosition
    {
        auto output = IndexData::MemoryPosition{0};

        if (false == db.Exists(table_, tsv(key_))) {
            db.Store(table_, tsv(key_), tsv(output));

            return output;
        }

        auto cb = [&output](const auto in) {
            if (sizeof(output) != in.size()) { return; }

            std::memcpy(&output, in.data(), in.size());
        };

        db.Load(table_, tsv(key_), cb);

        return output;
    }
    auto update_next_position(
        IndexData::MemoryPosition position,
        LMDB::Transaction& tx) noexcept -> bool
    {
        auto result = lmdb_.Store(table_, tsv(key_), tsv(position), tx);

        if (false == result.first) {
            LogOutput(OT_METHOD)(__func__)(": Failed to next write position")
                .Flush();

            return {};
        }

        next_position_ = position;

        return true;
    }

    Imp(opentxs::storage::lmdb::LMDB& lmdb,
        const std::string& basePath,
        const std::string filenamePrefix,
        int table,
        std::size_t key) noexcept(false)
        : lmdb_(lmdb)
        , path_prefix_(basePath)
        , filename_prefix_(filenamePrefix)
        , table_(table)
        , key_(key)
        , next_position_(load_position(lmdb_))
        , files_(init_files(path_prefix_, next_position_))
    {
        static_assert(1 == get_file_count(0));
        static_assert(1 == get_file_count(1));
        static_assert(1 == get_file_count(target_file_size_ - 1u));
        static_assert(2 == get_file_count(target_file_size_));
        static_assert(2 == get_file_count(target_file_size_ + 1u));
        static_assert(4 == get_file_count(3u * target_file_size_));
        static_assert(Offset{0, 0} == get_offset(0));
        static_assert(
            Offset{0, target_file_size_ - 1u} ==
            get_offset(target_file_size_ - 1u));
        static_assert(Offset{1, 0} == get_offset(target_file_size_));
        static_assert(Offset{1, 1} == get_offset(target_file_size_ + 1u));
        static_assert(0 == get_start_position(0));
        static_assert(target_file_size_ == get_start_position(1));

        {
            const auto offset = get_offset(next_position_);

            OT_ASSERT(files_.size() == (offset.first + 1));

            check_file(offset.first);

            OT_ASSERT(files_.size() == (offset.first + 1));
        }
    }
};

MappedFileStorage::MappedFileStorage(
    opentxs::storage::lmdb::LMDB& lmdb,
    const std::string& basePath,
    const std::string filenamePrefix,
    int table,
    std::size_t key) noexcept(false)
    : lmdb_(lmdb)
    , imp_p_(std::make_unique<Imp>(lmdb, basePath, filenamePrefix, table, key))
    , imp_(*imp_p_)
{
    OT_ASSERT(imp_p_);
}

auto MappedFileStorage::get_read_view(const IndexData& index) const noexcept
    -> ReadView
{
    return imp_.get_read_view(index);
}

auto MappedFileStorage::get_write_view(
    LMDB::Transaction& tx,
    IndexData& existing,
    UpdateCallback&& cb,
    std::size_t size) const noexcept -> WritableView
{
    return imp_.get_write_view(tx, existing, std::move(cb), size);
}

auto MappedFileStorage::get_write_view(
    LMDB::Transaction& tx,
    IndexData& index,
    std::size_t size) const noexcept -> WritableView
{
    return imp_.get_write_view(tx, index, {}, size);
}

MappedFileStorage::~MappedFileStorage() = default;
}  // namespace opentxs::util

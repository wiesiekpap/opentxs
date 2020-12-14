// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#if OPENTXS_BLOCK_STORAGE_ENABLED
#include "api/client/blockchain/database/Blocks.hpp"  // IWYU pragma: associated

#include <boost/filesystem.hpp>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>

#include "api/client/blockchain/database/Database.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "util/LMDB.hpp"

#define OT_METHOD                                                              \
    "opentxs::api::client::blockchain::database::implementation::Blocks::"

namespace fs = boost::filesystem;

namespace opentxs::api::client::blockchain::database::implementation
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

constexpr auto KiB_ = std::size_t{1024u};
constexpr auto MiB_ = std::size_t{1024u * KiB_};
constexpr auto GiB_ = std::size_t{1024u * MiB_};
[[maybe_unused]] constexpr auto TiB_ = std::size_t{1024u * GiB_};
constexpr auto target_file_size_ =
#if OT_VALGRIND || defined _WIN32 || defined __APPLE__
    std::size_t{1u * GiB_};
#else
    std::size_t{8u * TiB_};
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

const std::size_t Blocks::address_key_{
    static_cast<std::size_t>(Database::Key::NextBlockAddress)};

Blocks::Blocks(
    opentxs::storage::lmdb::LMDB& lmdb,
    const std::string& path) noexcept(false)
    : lmdb_(lmdb)
    , path_prefix_(path)
    , next_position_(load_position(lmdb_))
    , files_(init_files(path_prefix_, next_position_))
    , lock_()
    , block_locks_()
{
    static_assert(sizeof(std::uint64_t) == sizeof(std::size_t));
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
        Lock lock(lock_);
        const auto offset = get_offset(next_position_);

        OT_ASSERT(files_.size() == (offset.first + 1));

        check_file(lock, offset.first);

        OT_ASSERT(files_.size() == (offset.first + 1));
    }
}

auto Blocks::calculate_file_name(
    const std::string& prefix,
    const FileCounter index) noexcept -> std::string
{
    auto number = std::to_string(index);

    while (5 > number.size()) { number.insert(0, 1, '0'); }

    const auto filename = std::string{"blk"} + number + ".dat";
    auto path = fs::path{prefix};
    path /= filename;

    return path.string();
}

auto Blocks::check_file(const Lock& lock, const FileCounter position)
    const noexcept -> void
{
    while (files_.size() < (position + 1)) {
        create_or_load(path_prefix_, files_.size(), files_);
    }
}

auto Blocks::create_or_load(
    const std::string& prefix,
    const FileCounter file,
    std::vector<boost::iostreams::mapped_file>& output) noexcept -> void
{
    auto params =
        boost::iostreams::mapped_file_params{calculate_file_name(prefix, file)};
    params.flags = boost::iostreams::mapped_file::readwrite;
    const auto& path = params.path;
    LogTrace(OT_METHOD)(__FUNCTION__)(": initializing file ")(path).Flush();

    try {
        if (fs::exists(path)) {
            if (target_file_size_ == fs::file_size(path)) {
                params.new_file_size = 0;
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect size for ")(
                    path)
                    .Flush();
                fs::remove(path);
                params.new_file_size = target_file_size_;
            }
        } else {
            params.new_file_size = target_file_size_;
        }
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        OT_FAIL;
    }

    LogInsane(OT_METHOD)(__FUNCTION__)(": new_file_size: ")(
        params.new_file_size)
        .Flush();

    try {
        output.emplace_back(params);
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        OT_FAIL;
    }
}

auto Blocks::Exists(const Hash& block) const noexcept -> bool
{
    return lmdb_.Exists(Table::BlockIndex, block.Bytes());
}

auto Blocks::init_files(
    const std::string& prefix,
    const MemoryPosition position) noexcept
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

auto Blocks::Load(const Hash& block) const noexcept -> BlockReader
{
    Lock lock(lock_);
    auto index = IndexData{};
    auto cb = [&index](const auto in) {
        if (sizeof(index) != in.size()) { return; }

        std::memcpy(static_cast<void*>(&index), in.data(), in.size());
    };
    lmdb_.Load(Table::BlockIndex, block.Bytes(), cb);

    if (0 == index.size_) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Block ")(block.asHex())(
            " not found in index")
            .Flush();

        return {};
    }

    const auto [file, offset] = get_offset(index.position_);
    check_file(lock, file);

    return BlockReader{
        ReadView{files_.at(file).const_data() + offset, index.size_},
        block_locks_[block]};
}

auto Blocks::load_position([
    [maybe_unused]] opentxs::storage::lmdb::LMDB& db) noexcept -> MemoryPosition
{
    auto output = MemoryPosition{0};

    if (false == db.Exists(Table::Config, tsv(address_key_))) {
        db.Store(Config, tsv(address_key_), tsv(output));

        return output;
    }

    auto cb = [&output](const auto in) {
        if (sizeof(output) != in.size()) { return; }

        std::memcpy(&output, in.data(), in.size());
    };

    db.Load(Table::Config, tsv(address_key_), cb);

    return output;
}

auto Blocks::Store(const Hash& block, const std::size_t bytes) const noexcept
    -> BlockWriter
{
    if (0 == bytes) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Block ")(block.asHex())(
            " invalid block size")
            .Flush();

        return {};
    }

    Lock lock(lock_);
    auto index = IndexData{};
    auto cb = [&index](const auto in) {
        if (sizeof(index) != in.size()) { return; }

        std::memcpy(static_cast<void*>(&index), in.data(), in.size());
    };
    lmdb_.Load(Table::BlockIndex, block.Bytes(), cb);
    const auto replace = bytes == index.size_;

    if (replace) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Replacing existing block ")(
            block.asHex())
            .Flush();
    } else {
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

        LogVerbose(OT_METHOD)(__FUNCTION__)(": Storing block ")(block.asHex())(
            " at position ")(index.position_)
            .Flush();
    }

    const auto [file, offset] = get_offset(index.position_);
    check_file(lock, file);
    auto output = BlockWriter{
        WritableView{files_.at(file).data() + offset, bytes},
        block_locks_[block]};

    if (replace) { return output; }

    const auto nextPosition = index.position_ + bytes;
    auto tx = lmdb_.TransactionRW();
    auto result = lmdb_.Store(Table::BlockIndex, block.Bytes(), tsv(index), tx);

    if (false == result.first) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to update index for block ")(block.asHex())
            .Flush();

        return {};
    }

    result =
        lmdb_.Store(Table::Config, tsv(address_key_), tsv(nextPosition), tx);

    if (false == result.first) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to next write position")
            .Flush();

        return {};
    }

    if (tx.Finalize(true)) {
        next_position_ = nextPosition;

        return output;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Database update error").Flush();

        return {};
    }
}
}  // namespace opentxs::api::client::blockchain::database::implementation
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED

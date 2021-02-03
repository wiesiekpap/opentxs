// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "api/client/blockchain/database/Blocks.hpp"  // IWYU pragma: associated

#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

#include "api/client/blockchain/database/Database.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD                                                              \
    "opentxs::api::client::blockchain::database::implementation::Blocks::"

namespace opentxs::api::client::blockchain::database::implementation
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

Blocks::Blocks(
    opentxs::storage::lmdb::LMDB& lmdb,
    const std::string& path) noexcept(false)
    : MappedFileStorage(
          lmdb,
          path,
          "blk",
          Table::Config,
          static_cast<std::size_t>(Database::Key::NextBlockAddress))
    , table_(Table::BlockIndex)
    , lock_()
    , block_locks_()
{
}

auto Blocks::Exists(const Hash& block) const noexcept -> bool
{
    return lmdb_.Exists(table_, block.Bytes());
}

auto Blocks::Load(const Hash& block) const noexcept -> BlockReader
{
    auto lock = Lock{lock_};
    auto index = IndexData{};
    auto cb = [&index](const auto in) {
        if (sizeof(index) != in.size()) { return; }

        std::memcpy(static_cast<void*>(&index), in.data(), in.size());
    };
    lmdb_.Load(table_, block.Bytes(), cb);

    if (0 == index.size_) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Block ")(block.asHex())(
            " not found in index")
            .Flush();

        return {};
    }

    return BlockReader{get_read_view(index), block_locks_[block]};
}

auto Blocks::Store(const Hash& block, const std::size_t bytes) const noexcept
    -> BlockWriter
{
    auto lock = Lock{lock_};

    if (0 == bytes) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Block ")(block.asHex())(
            " invalid block size")
            .Flush();

        return {};
    }

    auto index = [&] {
        auto output = IndexData{};
        auto cb = [&output](const auto in) {
            if (sizeof(output) != in.size()) { return; }

            std::memcpy(static_cast<void*>(&output), in.data(), in.size());
        };
        lmdb_.Load(table_, block.Bytes(), cb);

        return output;
    }();
    auto cb = [&](auto& tx) -> bool {
        const auto result = lmdb_.Store(table_, block.Bytes(), tsv(index), tx);

        if (false == result.first) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to update index for block ")(block.asHex())
                .Flush();

            return false;
        }

        return true;
    };
    auto view = get_write_view(index, std::move(cb), bytes);

    if (false == view.valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to get write position for block ")(block.asHex())
            .Flush();

        return {};
    }

    return BlockWriter{std::move(view), block_locks_[block]};
}
}  // namespace opentxs::api::client::blockchain::database::implementation

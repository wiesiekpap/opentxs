// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "api/client/blockchain/database/Sync.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <algorithm>
#include <cstring>
#include <iosfwd>
#include <limits>
#include <memory>
#include <set>
#include <string_view>
#include <utility>

extern "C" {
#include <sodium.h>
}

#include "api/client/blockchain/database/Database.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/client/FilterOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/protobuf/BlockchainP2PSync.pb.h"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/verify/BlockchainP2PSync.hpp"
#include "util/ByteLiterals.hpp"
#include "util/LMDB.hpp"

#define OT_METHOD                                                              \
    "opentxs::api::client::blockchain::database::implementation::Sync::"

namespace opentxs::api::client::blockchain::database::implementation
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

const std::array<unsigned char, 16> Sync::checksum_key_{};

Sync::Sync(
    const api::Core& api,
    opentxs::storage::lmdb::LMDB& lmdb,
    const std::string& path) noexcept(false)
    : MappedFileStorage(
          lmdb,
          path,
          "sync",
          Table::Config,
          static_cast<std::size_t>(Database::Key::NextSyncAddress))
    , api_(api)
    , tip_table_(Table::SyncTips)
    , lock_()
    , tips_([&] {
        auto output = Tips{};

        for (const auto chain : opentxs::blockchain::DefinedChains()) {
            output.emplace(chain, -1);
        }

        return output;
    }())
{
    auto cb = [&](const auto key, const auto value) {
        auto chain = std::size_t{};
        auto height = Height{};

        if (key.size() != sizeof(chain)) {
            throw std::runtime_error("Invalid key");
        }

        if (value.size() != sizeof(height)) {
            throw std::runtime_error("Invalid value");
        }

        std::memcpy(&chain, key.data(), key.size());
        std::memcpy(&height, value.data(), value.size());
        tips_.at(static_cast<Chain>(chain)) = height;

        return true;
    };
    lmdb_.Read(tip_table_, cb, LMDB::Dir::Forward);

    static_assert(checksum_key_.size() == crypto_shorthash_KEYBYTES);

    for (const auto chain : opentxs::blockchain::SupportedChains()) {
        import_genesis(chain);
    }

    import_genesis(Chain::UnitTest);
}

auto Sync::import_genesis(const Chain chain) noexcept -> void
{
    if (0 <= tips_.at(chain)) { return; }

    const auto items = [&] {
        using Params = opentxs::blockchain::params::Data;
        const auto& data = Params::Chains().at(chain);
        constexpr auto filterType = opentxs::blockchain::filter::Type::ES;
        auto gcs = [&] {
            const auto& filter = Params::Filters().at(chain).at(filterType);
            const auto bytes =
                api_.Factory().Data(filter.second, StringStyle::Hex);
            const auto blockHash =
                api_.Factory().Data(data.genesis_hash_hex_, StringStyle::Hex);
            auto output =
                std::unique_ptr<const opentxs::blockchain::client::GCS>{
                    factory::GCS(
                        api_,
                        filterType,
                        opentxs::blockchain::internal::BlockHashToFilterKey(
                            blockHash->Bytes()),
                        bytes->Bytes())};

            OT_ASSERT(output);

            return output;
        }();
        auto output = Items{};
        auto& item = output.emplace_back();
        item.set_version(opentxs::blockchain::client::sync_data_version_);
        item.set_chain(static_cast<std::uint32_t>(chain));
        item.set_height(0);
        item.set_header(api_.Factory()
                            .Data(data.genesis_header_hex_, StringStyle::Hex)
                            ->str());
        item.set_filter_type(static_cast<std::uint32_t>(filterType));
        item.set_filter_element_count(gcs->ElementCount());
        item.set_filter(std::string{reader(gcs->Compressed())});

        return output;
    }();
    Store(chain, items);
}

auto Sync::Load(const Chain chain, const Height height, zmq::Message& output)
    const noexcept -> bool
{
    const auto start = static_cast<std::size_t>(height + 1);
    auto haveOne{false};
    auto total = std::size_t{};
    auto lock = SharedLock{lock_};
    const auto cb = [&](const auto key, const auto value) {
        if ((nullptr == key.data()) || (sizeof(std::size_t) != key.size())) {
            throw std::runtime_error("Invalid key");
        }

        const auto height = [&] {
            auto output = std::size_t{};
            std::memcpy(&output, key.data(), key.size());

            return output;
        }();

        try {
            const auto data = Data{value};
            const auto view = get_read_view(data.index_);

            if ((nullptr == view.data()) || (0 == view.size())) {
                throw std::runtime_error("Failed to load sync packet");
            }

            auto checksum = std::uint64_t{};

            static_assert(sizeof(checksum) == crypto_shorthash_BYTES);

            if (0 != ::crypto_shorthash(
                         reinterpret_cast<unsigned char*>(&checksum),
                         reinterpret_cast<const unsigned char*>(view.data()),
                         view.size(),
                         checksum_key_.data())) {
                throw std::runtime_error("Failed to calculate checksum");
            }

            if (data.checksum_ != checksum) {
                auto exclusive = boost::upgrade_to_unique_lock<Mutex>{lock};
                reorg(chain, height - 1);
                throw std::runtime_error("checksum failure");
            }

            output.AddFrame(view.data(), view.size());
            haveOne = true;
            total += view.size();

            return total < 1_MiB;
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

            return false;
        }
    };

    try {
        using Dir = opentxs::storage::lmdb::LMDB::Dir;
        lmdb_.ReadFrom(ChainToSyncTable(chain), start, cb, Dir::Forward);
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
    }

    return haveOne;
}

auto Sync::Reorg(const Chain chain, const Height height) const noexcept -> bool
{
    auto lock = ExclusiveLock{lock_};

    return reorg(chain, height);
}

auto Sync::reorg(const Chain chain, const Height height) const noexcept -> bool
{
    if (0 > height) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid height").Flush();

        return false;
    }

    auto& tip = tips_.at(chain);
    auto txn = lmdb_.TransactionRW();
    const auto table = ChainToSyncTable(chain);

    for (auto key = Height{height + 1}; key <= tip; ++key) {
        if (false == lmdb_.Delete(table, static_cast<std::size_t>(key), txn)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Delete error").Flush();

            return false;
        }
    }

    const auto key = static_cast<std::size_t>(chain);

    if (false == lmdb_.Store(tip_table_, key, tsv(height), txn).first) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to update tip").Flush();

        return false;
    }

    if (false == txn.Finalize(true)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Finalize error").Flush();

        return false;
    }

    tip = height;

    return true;
}

auto Sync::Store(const Chain chain, const Items& items) const noexcept -> bool
{
    if (0 == items.size()) { return true; }

    for (const auto& item : items) {
        if (false == proto::Validate(item, VERBOSE)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid protobuf").Flush();

            return false;
        }

        if (item.chain() != static_cast<std::uint32_t>(chain)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid chain").Flush();

            return false;
        }

        if (item.height() > std::numeric_limits<Height>::max()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid height").Flush();

            return false;
        }
    }

    auto lock = ExclusiveLock{lock_};

    if (const auto& first = items.front();
        static_cast<Height>(first.height()) <= tips_.at(chain)) {
        const auto parent = std::max<Height>(first.height() - 1, 0);

        if (false == reorg(chain, parent)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Reorg error").Flush();

            return false;
        }
    }

    auto previous = tips_.at(chain);

    OT_ASSERT(-2 < previous);

    auto txn = lmdb_.TransactionRW();
    LogTrace(OT_METHOD)(__FUNCTION__)(": previous tip height: ")(previous)
        .Flush();

    for (const auto& item : items) {
        if (static_cast<std::uint64_t>(++previous) != item.height()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": sequence error. Got ")(
                item.height())(" expected ")(previous)
                .Flush();

            return false;
        }

        const auto dbKey = static_cast<std::size_t>(item.height());
        auto data = Data{};
        const auto size = item.ByteSizeLong();

        if (0 >= size) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid serialized size")
                .Flush();

            return false;
        }

        auto write = get_write_view(txn, data.index_, size);

        if (false == write.valid(size)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to allocate space for writing")
                .Flush();

            return false;
        }

        item.SerializeWithCachedSizesToArray(write.as<std::uint8_t>());

        static_assert(sizeof(data.checksum_) == crypto_shorthash_BYTES);

        if (0 != ::crypto_shorthash(
                     data.WriteChecksum(),
                     write.as<unsigned char>(),
                     write.size(),
                     checksum_key_.data())) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate checksum")
                .Flush();

            return false;
        }

        const auto result =
            lmdb_.Store(ChainToSyncTable(chain), dbKey, data, txn);

        if (false == result.first) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to update index")
                .Flush();

            return false;
        }
    }

    const auto dbKey = static_cast<std::size_t>(chain);
    const auto tip = static_cast<Height>(items.back().height());

    if (false == lmdb_.Store(tip_table_, dbKey, tsv(tip), txn).first) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to update tip").Flush();

        return false;
    }

    tips_.at(chain) = tip;

    if (false == txn.Finalize(true)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Finalize error").Flush();

        return false;
    }

    return true;
}

auto Sync::Tip(const Chain chain) const noexcept -> Height
{
    auto lock = SharedLock{lock_};

    try {

        return static_cast<Height>(tips_.at(chain));
    } catch (...) {

        return -1;
    }
}
}  // namespace opentxs::api::client::blockchain::database::implementation

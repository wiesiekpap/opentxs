// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/thread/thread.hpp>
#include <array>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/protobuf/BlockchainP2PSync.pb.h"
#include "util/LMDB.hpp"
#include "util/MappedFileStorage.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainP2PSync;
}  // namespace proto

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::client::blockchain::database::implementation
{
class Sync final : private util::MappedFileStorage
{
public:
    using Chain = opentxs::blockchain::Type;
    using Height = opentxs::blockchain::block::Height;
    using Items = std::vector<proto::BlockchainP2PSync>;

    auto Load(const Chain chain, const Height height, zmq::Message& output)
        const noexcept -> bool;
    // Delete all entries with a height greater than specified
    auto Reorg(const Chain chain, const Height height) const noexcept -> bool;
    auto Store(const Chain chain, const Items& items) const noexcept -> bool;

    Sync(
        const api::Core& api,
        opentxs::storage::lmdb::LMDB& lmdb,
        const std::string& path) noexcept(false);

private:
    using Mutex = boost::upgrade_mutex;
    using SharedLock = boost::upgrade_lock<Mutex>;
    using ExclusiveLock = boost::unique_lock<Mutex>;
    using Tips = std::map<Chain, Height>;

    struct Data {
        IndexData index_;
        std::uint64_t checksum_;

        operator ReadView() const noexcept
        {
            return {reinterpret_cast<const char*>(this), sizeof(*this)};
        }

        auto WriteChecksum() noexcept
        {
            using Return = unsigned char;

            return reinterpret_cast<Return*>(&checksum_);
        }

        Data() noexcept
            : index_()
            , checksum_()
        {
        }
        Data(const ReadView in) noexcept(false)
            : Data()
        {
            if (in.size() != sizeof(*this)) {
                throw std::out_of_range("Invalid input data");
            }

            auto it = in.data();
            std::memcpy(&index_, it, sizeof(index_));
            std::advance(it, sizeof(index_));
            std::memcpy(&checksum_, it, sizeof(checksum_));
        }
    };

    static const std::array<unsigned char, 16> checksum_key_;

    const api::Core& api_;
    const int tip_table_;
    mutable Mutex lock_;
    mutable Tips tips_;

    auto import_genesis(const Chain chain) noexcept -> void;
    // WARNING make sure an exclusive lock is held
    auto reorg(const Chain chain, const Height height) const noexcept -> bool;
};
}  // namespace opentxs::api::client::blockchain::database::implementation

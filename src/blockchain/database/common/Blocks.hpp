// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "util/LMDB.hpp"
#include "util/MappedFileStorage.hpp"

namespace opentxs
{
namespace blockchain
{
namespace database
{
namespace common
{
class Bulk;
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

namespace opentxs::blockchain::database::common
{
class Blocks
{
public:
    using Hash = opentxs::blockchain::block::Hash;
    using pHash = opentxs::blockchain::block::pHash;

    auto Exists(const Hash& block) const noexcept -> bool;
    auto Load(const Hash& block) const noexcept -> BlockReader;
    auto Store(const Hash& block, const std::size_t bytes) const noexcept
        -> BlockWriter;

    Blocks(storage::lmdb::LMDB& lmdb, Bulk& bulk) noexcept;

private:
    storage::lmdb::LMDB& lmdb_;
    Bulk& bulk_;
    const int table_;
    mutable std::mutex lock_;
    mutable std::map<pHash, std::shared_mutex> block_locks_;
};
}  // namespace opentxs::blockchain::database::common

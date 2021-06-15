// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "api/client/blockchain/database/Database.hpp"

#pragma once

#include <cstdint>
#include <shared_mutex>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace database
{
namespace implementation
{
class Database;
}  // namespace implementation
}  // namespace database
}  // namespace blockchain
}  // namespace client
}  // namespace api
}  // namespace opentxs

namespace opentxs::blockchain::database
{
using BlockReader = ProtectedView<ReadView, std::shared_mutex, sLock>;
using BlockWriter = ProtectedView<WritableView, std::shared_mutex, eLock>;

enum Table {
    Config = 0,
    BlockHeaderMetadata = 1,
    BlockHeaderBest = 2,
    ChainData = 3,
    BlockHeaderSiblings = 4,
    BlockHeaderDisconnected = 5,
    BlockFilterBest = 6,
    BlockFilterHeaderBest = 7,
};

enum class Key : std::size_t {
    Version = 0,
    TipHeight = 1,
    CheckpointHeight = 2,
    CheckpointHash = 3,
    BestFullBlock = 4,
    SyncPosition = 5,
};

enum class BlockStorage : std::uint8_t {
    None = 0,
    Cache = 1,
    All = 2,
};
}  // namespace opentxs::blockchain::database

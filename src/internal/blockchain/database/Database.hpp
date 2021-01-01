// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "api/client/blockchain/database/Database.hpp"

namespace opentxs::blockchain::database
{
using Common = api::client::blockchain::database::implementation::Database;

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
}  // namespace opentxs::blockchain::database

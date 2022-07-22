// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <string_view>

#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::filteroracle
{
using NotifyCallback =
    std::function<void(const cfilter::Type, const block::Position&)>;

// WARNING update print function if new values are added or removed
enum class BlockIndexerJob : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    header = value(WorkType::BlockchainNewHeader),
    reorg = value(WorkType::BlockchainReorg),
    reindex = OT_ZMQ_INTERNAL_SIGNAL + 0,
    full_block = OT_ZMQ_NEW_FULL_BLOCK_SIGNAL,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

auto print(BlockIndexerJob) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node::filteroracle

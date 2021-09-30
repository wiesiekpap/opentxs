// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_NODE_TXOSTATE_HPP
#define OPENTXS_BLOCKCHAIN_NODE_TXOSTATE_HPP

#include "opentxs/blockchain/node/Types.hpp"  // IWYU pragma: associated

#include <limits>

namespace opentxs
{
namespace blockchain
{
namespace node
{
enum class TxoState : std::uint16_t {
    UnconfirmedNew = 0,
    UnconfirmedSpend = 1,
    ConfirmedNew = 2,
    ConfirmedSpend = 3,
    OrphanedNew = 4,
    OrphanedSpend = 5,
    Immature = 6,
    All = std::numeric_limits<std::uint16_t>::max(),
};
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
#endif

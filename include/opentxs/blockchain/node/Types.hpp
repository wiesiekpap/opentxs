// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/SendResult.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <utility>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node
{
enum class TxoState : std::uint16_t;
enum class TxoTag : std::uint16_t;

using SendOutcome = std::pair<SendResult, block::pTxid>;
}  // namespace opentxs::blockchain::node

namespace opentxs
{
OPENTXS_EXPORT auto print(blockchain::node::TxoState) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto print(blockchain::node::TxoTag) noexcept
    -> UnallocatedCString;
}  // namespace opentxs

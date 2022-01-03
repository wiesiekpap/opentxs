// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>

namespace opentxs
{
namespace blockchain
{
namespace node
{
enum class TxoState : std::uint16_t;
enum class TxoTag : std::uint16_t;
}  // namespace node
}  // namespace blockchain

OPENTXS_EXPORT auto print(blockchain::node::TxoState) noexcept -> std::string;
OPENTXS_EXPORT auto print(blockchain::node::TxoTag) noexcept -> std::string;
}  // namespace opentxs

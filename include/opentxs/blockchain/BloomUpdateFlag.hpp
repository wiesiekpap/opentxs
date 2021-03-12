// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOOMUPDATEFLAG_HPP
#define OPENTXS_BLOCKCHAIN_BLOOMUPDATEFLAG_HPP

#include "opentxs/blockchain/Types.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace blockchain
{
enum class BloomUpdateFlag : std::uint8_t { None = 0, All = 1, PubkeyOnly = 2 };
}  // namespace blockchain
}  // namespace opentxs
#endif

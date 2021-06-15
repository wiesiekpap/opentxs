// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_SUBCHAIN_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_SUBCHAIN_HPP

#include "opentxs/blockchain/crypto/Types.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
enum class Subchain : std::uint8_t {
    Error = 0,
    Internal = 1,
    External = 2,
    Incoming = 3,
    Outgoing = 4,
    Notification = 5,
    None = 255,
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif

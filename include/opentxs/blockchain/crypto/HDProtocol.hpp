// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_HDPROTOCOL_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_HDPROTOCOL_HPP

#include "opentxs/blockchain/crypto/Types.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
enum class HDProtocol : std::uint16_t {
    Error = 0,
    BIP_32 = 32,
    BIP_44 = 44,
    BIP_49 = 49,
    BIP_84 = 84,
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif

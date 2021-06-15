// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_ADDRESSSTYLE_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_ADDRESSSTYLE_HPP

#include "opentxs/blockchain/crypto/Types.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
enum class AddressStyle : std::uint16_t {
    Unknown = 0,
    P2PKH = 1,
    P2SH = 2,
    P2WPKH = 3,
    P2WSH = 4,
    P2TR = 5,
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif

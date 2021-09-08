// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_TYPES_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_TYPES_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
class EllipticCurve;
class HD;
}  // namespace key
}  // namespace crypto

namespace blockchain
{
namespace crypto
{
enum class AddressStyle : std::uint16_t;
enum class HDProtocol : std::uint16_t;
enum class SubaccountType : std::uint16_t;
enum class Subchain : std::uint8_t;

/// transaction id, output index
using Coin = std::pair<std::string, std::size_t>;
using ECKey = std::shared_ptr<const opentxs::crypto::key::EllipticCurve>;
using HDKey = std::shared_ptr<const opentxs::crypto::key::HD>;
/// account id, chain, index
using Key = std::tuple<std::string, Subchain, Bip32Index>;
using Activity = std::tuple<Coin, Key, Amount>;
}  // namespace crypto
}  // namespace blockchain

auto operator==(
    const blockchain::crypto::Key& lhs,
    const blockchain::crypto::Key& rhs) noexcept -> bool;
auto operator!=(
    const blockchain::crypto::Key& lhs,
    const blockchain::crypto::Key& rhs) noexcept -> bool;
auto print(blockchain::crypto::HDProtocol) noexcept -> std::string;
auto print(blockchain::crypto::Subchain) noexcept -> std::string;
auto print(const blockchain::crypto::Key&) noexcept -> std::string;
}  // namespace opentxs
#endif

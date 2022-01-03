// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"

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
}  // namespace opentxs

namespace opentxs::blockchain::crypto
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
}  // namespace opentxs::blockchain::crypto

namespace std
{
template <>
struct hash<opentxs::blockchain::crypto::Key> {
    auto operator()(const opentxs::blockchain::crypto::Key& data) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs
{
OPENTXS_EXPORT auto operator==(
    const blockchain::crypto::Key& lhs,
    const blockchain::crypto::Key& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const blockchain::crypto::Key& lhs,
    const blockchain::crypto::Key& rhs) noexcept -> bool;
auto preimage(const blockchain::crypto::Key& in) noexcept -> Space;
OPENTXS_EXPORT auto print(blockchain::crypto::HDProtocol) noexcept
    -> std::string;
OPENTXS_EXPORT auto print(blockchain::crypto::Subchain) noexcept -> std::string;
OPENTXS_EXPORT auto print(const blockchain::crypto::Key&) noexcept
    -> std::string;
}  // namespace opentxs

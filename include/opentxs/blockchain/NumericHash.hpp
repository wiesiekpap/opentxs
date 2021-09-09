// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_NUMERIC_HASH_HPP
#define OPENTXS_BLOCKCHAIN_NUMERIC_HASH_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"

namespace opentxs
{
namespace blockchain
{
class NumericHash;
}  // namespace blockchain

using OTNumericHash = Pimpl<blockchain::NumericHash>;

OPENTXS_EXPORT auto operator==(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept -> bool;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
class OPENTXS_EXPORT NumericHash
{
public:
    static auto MaxTarget(const Type chain) noexcept -> std::int32_t;

    virtual auto operator==(const blockchain::NumericHash& rhs) const noexcept
        -> bool = 0;
    virtual auto operator!=(const blockchain::NumericHash& rhs) const noexcept
        -> bool = 0;
    virtual auto operator<(const blockchain::NumericHash& rhs) const noexcept
        -> bool = 0;
    virtual auto operator<=(const blockchain::NumericHash& rhs) const noexcept
        -> bool = 0;

    virtual auto asHex(const std::size_t minimumBytes = 32) const noexcept
        -> std::string = 0;
    virtual auto Decimal() const noexcept -> std::string = 0;

    virtual ~NumericHash() = default;

protected:
    NumericHash() noexcept = default;

private:
    friend OTNumericHash;

    virtual auto clone() const noexcept -> NumericHash* = 0;

    NumericHash(const NumericHash& rhs) = delete;
    NumericHash(NumericHash&& rhs) = delete;
    auto operator=(const NumericHash& rhs) -> NumericHash& = delete;
    auto operator=(NumericHash&& rhs) -> NumericHash& = delete;
};
}  // namespace blockchain
}  // namespace opentxs
#endif

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

OPENTXS_EXPORT bool operator==(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept;
OPENTXS_EXPORT bool operator!=(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept;
OPENTXS_EXPORT bool operator<(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept;
OPENTXS_EXPORT bool operator<=(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
class NumericHash
{
public:
    OPENTXS_EXPORT static std::int32_t MaxTarget(const Type chain) noexcept;

    OPENTXS_EXPORT virtual bool operator==(
        const blockchain::NumericHash& rhs) const noexcept = 0;
    OPENTXS_EXPORT virtual bool operator!=(
        const blockchain::NumericHash& rhs) const noexcept = 0;
    OPENTXS_EXPORT virtual bool operator<(
        const blockchain::NumericHash& rhs) const noexcept = 0;
    OPENTXS_EXPORT virtual bool operator<=(
        const blockchain::NumericHash& rhs) const noexcept = 0;

    OPENTXS_EXPORT virtual std::string asHex(
        const std::size_t minimumBytes = 32) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Decimal() const noexcept = 0;

    OPENTXS_EXPORT virtual ~NumericHash() = default;

protected:
    NumericHash() noexcept = default;

private:
    friend OTNumericHash;

    virtual NumericHash* clone() const noexcept = 0;

    NumericHash(const NumericHash& rhs) = delete;
    NumericHash(NumericHash&& rhs) = delete;
    NumericHash& operator=(const NumericHash& rhs) = delete;
    NumericHash& operator=(NumericHash&& rhs) = delete;
};
}  // namespace blockchain
}  // namespace opentxs
#endif

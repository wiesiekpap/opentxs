// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Amount;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace bmp = boost::multiprecision;

namespace opentxs::amount
{
static constexpr auto fractional_bits_{
    std::numeric_limits<std::uint64_t>::digits};
static constexpr auto minimum_integer_bits_{
    std::numeric_limits<std::uint64_t>::digits + fractional_bits_};
static constexpr auto maximum_integer_bits_{256u + fractional_bits_};

using Integer = bmp::number<bmp::cpp_int_backend<
    minimum_integer_bits_,
    maximum_integer_bits_,
    bmp::signed_magnitude,
    bmp::checked,
    void>>;
using Float = bmp::cpp_dec_float_100;

auto FloatToInteger(const Float& rhs) noexcept(false) -> Integer;
auto GetScale() noexcept -> Float;
auto IntegerToFloat(const Integer& rhs) noexcept -> Float;
}  // namespace opentxs::amount

namespace opentxs::internal
{
auto FloatToAmount(const amount::Float& rhs) noexcept(false)
    -> opentxs::Amount::Imp*;

class Amount
{
public:
    static auto SerializeBitcoinSize() noexcept -> std::size_t;

    virtual auto ExtractInt64() const noexcept(false) -> std::int64_t = 0;
    virtual auto ExtractUInt64() const noexcept(false) -> std::uint64_t = 0;
    virtual auto SerializeBitcoin(const AllocateOutput dest) const noexcept
        -> bool = 0;
    virtual auto ToFloat() const noexcept -> amount::Float = 0;

    Amount() noexcept
    {
        static_assert(amount::minimum_integer_bits_ == 128u);
        static_assert(amount::maximum_integer_bits_ == 320u);
    }
    Amount(const Amount&) noexcept = default;

    virtual ~Amount() = default;
};
}  // namespace opentxs::internal

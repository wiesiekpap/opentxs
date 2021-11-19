// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <string>
#include <string_view>

#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs
{
class Amount;

auto operator<(const int lhs, const Amount& rhs) noexcept -> bool;
auto operator<(const long int lhs, const Amount& rhs) noexcept -> bool;
auto operator<(const long long int lhs, const Amount& rhs) noexcept -> bool;
auto operator<(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool;

auto operator>(const int lhs, const Amount& rhs) noexcept -> bool;
auto operator>(const long int lhs, const Amount& rhs) noexcept -> bool;
auto operator>(const long long int lhs, const Amount& rhs) noexcept -> bool;
auto operator>(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool;

auto operator==(const int lhs, const Amount& rhs) noexcept -> bool;
auto operator==(const long int lhs, const Amount& rhs) noexcept -> bool;
auto operator==(const long long int lhs, const Amount& rhs) noexcept -> bool;
auto operator==(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool;

auto operator!=(const int lhs, const Amount& rhs) noexcept -> bool;
auto operator!=(const long long int lhs, const Amount& rhs) noexcept -> bool;
auto operator!=(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool;

auto operator<=(const long int lhs, const Amount& rhs) noexcept -> bool;
auto operator<=(const long long int lhs, const Amount& rhs) noexcept -> bool;
auto operator<=(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool;

auto operator>=(const int lhs, const Amount& rhs) noexcept -> bool;
auto operator>=(const long long int lhs, const Amount& rhs) noexcept -> bool;
auto operator>=(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool;

template <typename T>
auto operator*(T lhs, const Amount& rhs) noexcept(false) -> Amount;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT Amount
{
public:
    template <typename T>
    auto operator<(const T) const noexcept -> bool;
    auto operator<(const Amount& rhs) const noexcept -> bool;

    template <typename T>
    auto operator>(const T) const noexcept -> bool;
    auto operator>(const Amount& rhs) const noexcept -> bool;

    template <typename T>
    auto operator==(const T) const noexcept -> bool;
    auto operator==(const Amount& rhs) const noexcept -> bool;

    auto operator!=(const Amount& rhs) const noexcept -> bool;
    template <typename T>
    auto operator!=(const T) const noexcept -> bool;

    auto operator<=(const Amount& rhs) const noexcept -> bool;
    template <typename T>
    auto operator<=(const T) const noexcept -> bool;

    auto operator>=(const Amount& rhs) const noexcept -> bool;
    template <typename T>
    auto operator>=(const T) const noexcept -> bool;

    auto operator+(const Amount& rhs) const noexcept(false) -> Amount;

    auto operator-(const Amount& rhs) const noexcept(false) -> Amount;

    auto operator*(const Amount& rhs) const noexcept(false) -> Amount;
    template <typename T>
    auto operator*(const T) const noexcept(false) -> Amount;

    auto operator/(const Amount& rhs) const noexcept(false) -> Amount;
    template <typename T>
    auto operator/(const T) const noexcept(false) -> Amount;

    auto operator%(const Amount& rhs) const noexcept(false) -> Amount;
    template <typename T>
    auto operator%(const T) const noexcept(false) -> Amount;

    auto operator*=(const Amount& amount) noexcept(false) -> Amount&;

    auto operator+=(const Amount& amount) noexcept(false) -> Amount&;

    auto operator-=(const Amount& amount) noexcept(false) -> Amount&;

    auto operator-() -> Amount;

    auto Serialize(const AllocateOutput dest) const noexcept -> bool;

    OPENTXS_NO_EXPORT auto SerializeBitcoin(
        const AllocateOutput dest) const noexcept -> bool;
    OPENTXS_NO_EXPORT static auto SerializeBitcoinSize() noexcept
        -> std::size_t;

    static auto signed_amount(
        long long int ip,
        unsigned long long int fp = 0,
        unsigned long long int div = 0) -> Amount;
    static auto unsigned_amount(
        unsigned long long int ip,
        unsigned long long int fp = 0,
        unsigned long long int div = 0) -> Amount;

    struct Imp;

    OPENTXS_NO_EXPORT auto Internal() const noexcept -> Imp&;

    Amount(int);
    Amount(long int);
    Amount(long long int);
    Amount(unsigned int);
    Amount(unsigned long int);
    Amount(unsigned long long int);
    Amount(std::string_view str, bool normalize = false) noexcept(false);
    Amount(const opentxs::network::zeromq::Frame&);
    Amount() noexcept;
    Amount(const Amount& rhs) noexcept;
    Amount(Amount&& rhs) noexcept;
    auto operator=(const Amount&) -> Amount&;
    auto operator=(Amount&&) -> Amount&;

    ~Amount();

private:
    Amount(const Imp&);

    friend auto operator<(const int lhs, const Amount& rhs) noexcept -> bool;
    friend auto operator<(const long int lhs, const Amount& rhs) noexcept
        -> bool;
    friend auto operator<(const long long int lhs, const Amount& rhs) noexcept
        -> bool;
    friend auto operator<(
        const unsigned long long int lhs,
        const Amount& rhs) noexcept -> bool;

    friend auto operator>(const int lhs, const Amount& rhs) noexcept -> bool;
    friend auto operator>(const long int lhs, const Amount& rhs) noexcept
        -> bool;
    friend auto operator>(const long long int lhs, const Amount& rhs) noexcept
        -> bool;
    friend auto operator>(
        const unsigned long long int lhs,
        const Amount& rhs) noexcept -> bool;

    friend auto operator==(const int lhs, const Amount& rhs) noexcept -> bool;
    friend auto operator==(const long int lhs, const Amount& rhs) noexcept
        -> bool;
    friend auto operator==(const long long int lhs, const Amount& rhs) noexcept
        -> bool;
    friend auto operator==(
        const unsigned long long int lhs,
        const Amount& rhs) noexcept -> bool;

    friend auto operator!=(const int lhs, const Amount& rhs) noexcept -> bool;
    friend auto operator!=(const long long int lhs, const Amount& rhs) noexcept
        -> bool;
    friend auto operator!=(
        const unsigned long long int lhs,
        const Amount& rhs) noexcept -> bool;

    friend auto operator<=(const long int lhs, const Amount& rhs) noexcept
        -> bool;
    friend auto operator<=(const long long int lhs, const Amount& rhs) noexcept
        -> bool;
    friend auto operator<=(
        const unsigned long long int lhs,
        const Amount& rhs) noexcept -> bool;

    friend auto operator>=(const int lhs, const Amount& rhs) noexcept -> bool;

    friend auto operator>=(const long long int lhs, const Amount& rhs) noexcept
        -> bool;
    friend auto operator>=(
        const unsigned long long int lhs,
        const Amount& rhs) noexcept -> bool;

    template <typename T>
    friend auto operator*(const T lhs, const Amount& rhs) noexcept(false)
        -> Amount;

    Imp* imp_;
};
}  // namespace opentxs

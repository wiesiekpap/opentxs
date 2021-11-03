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

auto operator+(const long int lhs, const Amount& rhs) noexcept(false) -> Amount;
auto operator+(const long long int lhs, const Amount& rhs) noexcept(false)
    -> Amount;
auto operator+(const unsigned long int lhs, const Amount& rhs) noexcept(false)
    -> Amount;

auto operator-(const long int lhs, const Amount& rhs) noexcept(false) -> Amount;
auto operator-(const long long int lhs, const Amount& rhs) noexcept(false)
    -> Amount;

auto operator*(const int lhs, const Amount& rhs) noexcept(false) -> Amount;
auto operator*(const long long int lhs, const Amount& rhs) noexcept(false)
    -> Amount;
auto operator*(const unsigned int lhs, const Amount& rhs) noexcept(false)
    -> Amount;
auto operator*(const unsigned long int lhs, const Amount& rhs) noexcept(false)
    -> Amount;
auto operator*(const unsigned long long int lhs, const Amount& rhs) noexcept(
    false) -> Amount;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT Amount
{
public:
    auto operator<(const Amount& rhs) const noexcept -> bool;
    auto operator<(const int rhs) const noexcept -> bool;
    auto operator<(const long int rhs) const noexcept -> bool;
    auto operator<(const long long int rhs) const noexcept -> bool;
    auto operator<(const unsigned long long int rhs) const noexcept -> bool;

    auto operator>(const Amount& rhs) const noexcept -> bool;
    auto operator>(const int rhs) const noexcept -> bool;
    auto operator>(const long int rhs) const noexcept -> bool;
    auto operator>(const long long int rhs) const noexcept -> bool;
    auto operator>(const unsigned long long int rhs) const noexcept -> bool;

    auto operator==(const Amount& rhs) const noexcept -> bool;
    auto operator==(const int rhs) const noexcept -> bool;
    auto operator==(const long int rhs) const noexcept -> bool;
    auto operator==(const long long int rhs) const noexcept -> bool;
    auto operator==(const unsigned int rhs) const noexcept -> bool;
    auto operator==(const unsigned long int rhs) const noexcept -> bool;
    auto operator==(const unsigned long long int rhs) const noexcept -> bool;

    auto operator!=(const Amount& rhs) const noexcept -> bool;
    auto operator!=(const int rhs) const noexcept -> bool;
    auto operator!=(const long int rhs) const noexcept -> bool;
    auto operator!=(const long long int rhs) const noexcept -> bool;
    auto operator!=(const unsigned long long int rhs) const noexcept -> bool;

    auto operator<=(const Amount& rhs) const noexcept -> bool;
    auto operator<=(const int rhs) const noexcept -> bool;
    auto operator<=(const long long int rhs) const noexcept -> bool;
    auto operator<=(const unsigned int rhs) const noexcept -> bool;
    auto operator<=(const unsigned long int rhs) const noexcept -> bool;
    auto operator<=(const unsigned long long int rhs) const noexcept -> bool;

    auto operator>=(const Amount& rhs) const noexcept -> bool;
    auto operator>=(const long int rhs) const noexcept -> bool;
    auto operator>=(const long long int rhs) const noexcept -> bool;
    auto operator>=(const unsigned long long int rhs) const noexcept -> bool;

    auto operator+(const Amount& rhs) const noexcept(false) -> Amount;
    auto operator+(const long int rhs) const noexcept(false) -> Amount;
    auto operator+(const long long int rhs) const noexcept(false) -> Amount;
    auto operator+(const unsigned int rhs) const noexcept(false) -> Amount;
    auto operator+(const unsigned long int rhs) const noexcept(false) -> Amount;
    auto operator+(const unsigned long long int rhs) const noexcept(false)
        -> Amount;

    auto operator-(const Amount& rhs) const noexcept(false) -> Amount;
    auto operator-(const long int rhs) const noexcept(false) -> Amount;
    auto operator-(const long long int rhs) const noexcept(false) -> Amount;
    auto operator-(const unsigned long int rhs) const noexcept(false) -> Amount;
    auto operator-(const unsigned long long int rhs) const noexcept(false)
        -> Amount;

    auto operator*(const Amount& rhs) const noexcept(false) -> Amount;
    auto operator*(const int rhs) const noexcept(false) -> Amount;
    auto operator*(const long int rhs) const noexcept(false) -> Amount;
    auto operator*(const long long int rhs) const noexcept(false) -> Amount;
    auto operator*(const unsigned long long int rhs) const noexcept(false)
        -> Amount;

    auto operator/(const Amount& rhs) const noexcept(false) -> Amount;
    auto operator/(const int rhs) const noexcept(false) -> Amount;
    auto operator/(const long long int rhs) const noexcept(false) -> Amount;
    auto operator/(const unsigned long long int rhs) const noexcept(false)
        -> Amount;

    auto operator%(const Amount& rhs) const noexcept(false) -> Amount;
    auto operator%(const int rhs) const noexcept(false) -> Amount;
    auto operator%(const long long int rhs) const noexcept(false) -> Amount;
    auto operator%(const unsigned long long int rhs) const noexcept(false)
        -> Amount;

    auto operator*=(const Amount& amount) noexcept(false) -> Amount&;

    auto operator+=(const Amount& amount) noexcept(false) -> Amount&;
    auto operator+=(const unsigned long int amount) noexcept(false) -> Amount&;

    auto operator-=(const Amount& amount) noexcept(false) -> Amount&;
    auto operator-=(const unsigned long int amount) noexcept(false) -> Amount&;

    auto operator-() -> Amount;

    operator std::string() const noexcept(false) { return this->str(); }

    auto str() const -> std::string;

    OPENTXS_NO_EXPORT auto SerializeBitcoin(
        const AllocateOutput dest) const noexcept -> bool;
    OPENTXS_NO_EXPORT static auto SerializeBitcoinSize() noexcept
        -> std::size_t;

    struct Imp;

    OPENTXS_NO_EXPORT auto Internal() const noexcept -> Imp&;

    Amount(int);
    Amount(long int);
    Amount(long long int);
    Amount(unsigned int);
    Amount(unsigned long int);
    Amount(unsigned long long int);
    Amount(std::string_view str) noexcept(false);
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

    friend auto operator+(const long int lhs, const Amount& rhs) noexcept(false)
        -> Amount;
    friend auto operator+(const long long int lhs, const Amount& rhs) noexcept(
        false) -> Amount;
    friend auto operator+(
        const unsigned long int lhs,
        const Amount& rhs) noexcept(false) -> Amount;

    friend auto operator-(const long int lhs, const Amount& rhs) noexcept(false)
        -> Amount;
    friend auto operator-(const long long int lhs, const Amount& rhs) noexcept(
        false) -> Amount;

    friend auto operator*(const int lhs, const Amount& rhs) noexcept(false)
        -> Amount;
    friend auto operator*(const long long int lhs, const Amount& rhs) noexcept(
        false) -> Amount;
    friend auto operator*(const unsigned int lhs, const Amount& rhs) noexcept(
        false) -> Amount;
    friend auto operator*(
        const unsigned long int lhs,
        const Amount& rhs) noexcept(false) -> Amount;
    friend auto operator*(
        const unsigned long long int lhs,
        const Amount& rhs) noexcept(false) -> Amount;

    Imp* imp_;
};
}  // namespace opentxs

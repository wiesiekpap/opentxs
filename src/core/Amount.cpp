// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "opentxs/core/Amount.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <memory>

#include "core/Amount.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"

namespace be = boost::endian;

namespace opentxs
{
auto signed_amount(
    long long int ip,
    unsigned long long int fp,
    unsigned long long int div) -> Amount
{
    using Imp = Amount::Imp;

    return Amount(Imp::shift_left(ip) + (Imp::shift_left(fp) / div));
}

auto unsigned_amount(
    unsigned long long int ip,
    unsigned long long int fp,
    unsigned long long int div) -> Amount
{
    using Imp = Amount::Imp;

    return Amount(Imp::shift_left(ip) + (Imp::shift_left(fp) / div));
}

auto Amount::operator<(const Amount& rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ < *rhs.imp_;
}

template <typename T>
auto Amount::operator<(const T rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);

    return *imp_ < rhs;
}
// clang-format off
template auto opentxs::Amount::operator< <int>(const int rhs) const noexcept
    -> bool;
template auto opentxs::Amount::operator< <long>(const long rhs) const noexcept
    -> bool;
template auto opentxs::Amount::operator< <long long>(
    const long long rhs) const noexcept -> bool;
template auto opentxs::Amount::operator< <unsigned long long>(
    const unsigned long long rhs) const noexcept -> bool;
// clang-format on

auto Amount::operator>(const Amount& rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ > *rhs.imp_;
}

template <typename T>
auto Amount::operator>(const T rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);

    return *imp_ > rhs;
}

template auto opentxs::Amount::operator><int>(const int rhs) const noexcept
    -> bool;
template auto opentxs::Amount::operator><long>(const long rhs) const noexcept
    -> bool;
template auto opentxs::Amount::operator>
    <long long>(const long long rhs) const noexcept -> bool;
template auto opentxs::Amount::operator>
    <unsigned long long>(const unsigned long long rhs) const noexcept -> bool;

auto Amount::operator==(const Amount& rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ == *rhs.imp_;
}

template <typename T>
auto Amount::operator==(const T rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);

    return *imp_ == rhs;
}

template auto opentxs::Amount::operator==<int>(const int rhs) const noexcept
    -> bool;
template auto opentxs::Amount::operator==<long>(const long rhs) const noexcept
    -> bool;
template auto opentxs::Amount::operator==
    <long long>(const long long rhs) const noexcept -> bool;
template auto opentxs::Amount::operator==
    <unsigned int>(const unsigned int rhs) const noexcept -> bool;
template auto opentxs::Amount::operator==
    <unsigned long int>(const unsigned long int rhs) const noexcept -> bool;
template auto opentxs::Amount::operator==
    <unsigned long long>(const unsigned long long rhs) const noexcept -> bool;

auto Amount::operator!=(const Amount& rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ != *rhs.imp_;
}

template <typename T>
auto Amount::operator!=(const T rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);

    return *imp_ != rhs;
}

template auto opentxs::Amount::operator!=<int>(const int rhs) const noexcept
    -> bool;
template auto opentxs::Amount::operator!=<long>(const long rhs) const noexcept
    -> bool;
template auto opentxs::Amount::operator!=
    <long long>(const long long rhs) const noexcept -> bool;
template auto opentxs::Amount::operator!=
    <unsigned long long>(const unsigned long long rhs) const noexcept -> bool;

auto Amount::operator<=(const Amount& rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ <= *rhs.imp_;
}

template <typename T>
auto Amount::operator<=(const T rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);

    return *imp_ <= rhs;
}

template auto opentxs::Amount::operator<=<int>(const int rhs) const noexcept
    -> bool;
template auto opentxs::Amount::operator<=
    <long long>(const long long rhs) const noexcept -> bool;
template auto opentxs::Amount::operator<=
    <unsigned int>(const unsigned int rhs) const noexcept -> bool;
template auto opentxs::Amount::operator<=
    <unsigned long int>(const unsigned long int rhs) const noexcept -> bool;
template auto opentxs::Amount::operator<=
    <unsigned long long>(const unsigned long long rhs) const noexcept -> bool;

auto Amount::operator>=(const Amount& rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ >= *rhs.imp_;
}

template <typename T>
auto Amount::operator>=(const T rhs) const noexcept -> bool
{
    OT_ASSERT(imp_);

    return *imp_ >= rhs;
}

template auto opentxs::Amount::operator>=<long>(const long rhs) const noexcept
    -> bool;
template auto opentxs::Amount::operator>=
    <long long>(const long long rhs) const noexcept -> bool;
template auto opentxs::Amount::operator>=
    <unsigned long long>(const unsigned long long rhs) const noexcept -> bool;

auto Amount::operator+(const Amount& rhs) const noexcept(false) -> Amount
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ + *rhs.imp_;
}

auto Amount::operator-(const Amount& rhs) const noexcept(false) -> Amount
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ - *rhs.imp_;
}

auto Amount::operator*(const Amount& rhs) const noexcept(false) -> Amount
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ * *rhs.imp_;
}

template <typename T>
auto Amount::operator*(const T rhs) const noexcept(false) -> Amount
{
    OT_ASSERT(imp_);

    return *imp_ * rhs;
}

template auto Amount::operator*<int>(const int rhs) const noexcept(false)
    -> Amount;
template auto Amount::operator*<long>(const long rhs) const noexcept(false)
    -> Amount;
template auto Amount::operator*<long long>(const long long rhs) const
    noexcept(false) -> Amount;
template auto Amount::operator*
    <unsigned long long>(const unsigned long long rhs) const noexcept(false)
        -> Amount;

auto Amount::operator/(const Amount& rhs) const noexcept(false) -> Amount
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ / *rhs.imp_;
}

template <typename T>
auto Amount::operator/(const T rhs) const noexcept(false) -> Amount
{
    OT_ASSERT(imp_);

    return *imp_ / rhs;
}

template auto Amount::operator/<int>(const int rhs) const noexcept(false)
    -> Amount;
template auto Amount::operator/<long long>(const long long rhs) const
    noexcept(false) -> Amount;
template auto Amount::operator/
    <unsigned long long>(const unsigned long long rhs) const noexcept(false)
        -> Amount;

auto Amount::operator%(const Amount& rhs) const noexcept(false) -> Amount
{
    OT_ASSERT(imp_);
    OT_ASSERT(rhs.imp_);

    return *imp_ % *rhs.imp_;
}

template <typename T>
auto Amount::operator%(const T rhs) const noexcept(false) -> Amount
{
    OT_ASSERT(imp_);

    return *imp_ % rhs;
}

template auto Amount::operator%<int>(const int rhs) const noexcept(false)
    -> Amount;
template auto Amount::operator%<long long>(const long long rhs) const
    noexcept(false) -> Amount;
template auto Amount::operator%
    <unsigned long long>(const unsigned long long rhs) const noexcept(false)
        -> Amount;

auto Amount::operator*=(const Amount& amount) noexcept(false) -> Amount&
{
    OT_ASSERT(imp_);
    OT_ASSERT(amount.imp_);

    *imp_ *= *amount.imp_;

    return *this;
}

auto Amount::operator+=(const Amount& amount) noexcept(false) -> Amount&
{
    OT_ASSERT(imp_);
    OT_ASSERT(amount.imp_);

    *imp_ += *amount.imp_;

    return *this;
}

auto Amount::operator-=(const Amount& amount) noexcept(false) -> Amount&
{
    OT_ASSERT(imp_);
    OT_ASSERT(amount.imp_);

    *imp_ -= *amount.imp_;

    return *this;
}

auto Amount::operator-() -> Amount { return -*imp_; }

auto Amount::Serialize(const AllocateOutput dest) const noexcept -> bool
{
    OT_ASSERT(imp_);

    return imp_->Serialize(dest);
}

auto Amount::SerializeBitcoin(const AllocateOutput dest) const noexcept -> bool
{
    OT_ASSERT(imp_);

    return imp_->SerializeBitcoin(dest);
}

auto Amount::SerializeBitcoinSize() noexcept -> std::size_t
{
    return sizeof(be::little_int64_buf_t);
}

auto Amount::Internal() const noexcept -> Imp&
{
    OT_ASSERT(imp_);

    return *const_cast<Amount&>(*this).imp_;
}

Amount::Amount(const Imp& amount)
    : imp_(std::make_unique<Imp>(amount).release())
{
    OT_ASSERT(imp_);
}
Amount::Amount(int amount)
    : imp_(std::make_unique<Imp>(amount).release())
{
    OT_ASSERT(imp_);
}
Amount::Amount(long int amount)
    : imp_(std::make_unique<Imp>(amount).release())
{
    OT_ASSERT(imp_);
}
Amount::Amount(long long int amount)
    : imp_(std::make_unique<Imp>(amount).release())
{
    OT_ASSERT(imp_);
}
Amount::Amount(unsigned int amount)
    : imp_(std::make_unique<Imp>(amount).release())
{
    OT_ASSERT(imp_);
}
Amount::Amount(unsigned long int amount)
    : imp_(std::make_unique<Imp>(amount).release())
{
    OT_ASSERT(imp_);
}
Amount::Amount(unsigned long long int amount)
    : imp_(std::make_unique<Imp>(amount).release())
{
    OT_ASSERT(imp_);
}
Amount::Amount(std::string_view str, bool normalize) noexcept(false)
    : imp_(std::make_unique<Imp>(str, normalize).release())
{
    OT_ASSERT(imp_);
}
Amount::Amount(const network::zeromq::Frame& frame)
    : imp_(std::make_unique<Imp>(frame.Bytes()).release())
{
    OT_ASSERT(imp_);
}
Amount::Amount() noexcept
    : imp_(std::make_unique<Imp>().release())
{
    OT_ASSERT(imp_);
}
Amount::Amount(const Amount& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_).release())
{
    OT_ASSERT(imp_);
}
Amount::Amount(Amount&& rhs) noexcept
    : imp_(rhs.imp_)
{
    rhs.imp_ = nullptr;

    OT_ASSERT(imp_);
}

auto Amount::operator=(const Amount& amount) -> Amount&
{
    OT_ASSERT(imp_);
    OT_ASSERT(amount.imp_);

    *imp_ = *amount.imp_;
    return *this;
}

auto Amount::operator=(Amount&& amount) -> Amount&
{
    if (nullptr != imp_) { delete imp_; }
    if (nullptr != amount.imp_) {
        imp_ = amount.imp_;
        amount.imp_ = nullptr;
    } else {
        imp_ = std::make_unique<Imp>().release();
    }
    return *this;
}

Amount::~Amount()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}

auto operator<(const int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) < rhs.imp_->amount_;
}
auto operator<(const long int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) < rhs.imp_->amount_;
}
auto operator<(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) < rhs.imp_->amount_;
}
auto operator<(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) < rhs.imp_->amount_;
}

auto operator>(const int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) > rhs.imp_->amount_;
}
auto operator>(const long int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) > rhs.imp_->amount_;
}
auto operator>(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) > rhs.imp_->amount_;
}
auto operator>(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) > rhs.imp_->amount_;
}

auto operator==(const int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) == rhs.imp_->amount_;
}
auto operator==(const long int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) == rhs.imp_->amount_;
}
auto operator==(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) == rhs.imp_->amount_;
}
auto operator==(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) == rhs.imp_->amount_;
}

auto operator!=(const int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) != rhs.imp_->amount_;
}
auto operator!=(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) != rhs.imp_->amount_;
}
auto operator!=(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) != rhs.imp_->amount_;
}

auto operator<=(const long int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) <= rhs.imp_->amount_;
}
auto operator<=(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) <= rhs.imp_->amount_;
}
auto operator<=(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) <= rhs.imp_->amount_;
}

auto operator>=(const int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) >= rhs.imp_->amount_;
}
auto operator>=(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) >= rhs.imp_->amount_;
}
auto operator>=(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    OT_ASSERT(rhs.imp_);

    return Amount::Imp::shift_left(lhs) >= rhs.imp_->amount_;
}

template <typename T>
auto operator*(const T lhs, const Amount& rhs) noexcept(false) -> Amount
{
    OT_ASSERT(rhs.imp_);

    const auto total = lhs * rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}

template auto operator*<int>(const int lhs, const Amount& rhs) noexcept(false)
    -> Amount;
template auto operator*
    <long long>(const long long lhs, const Amount& rhs) noexcept(false)
        -> Amount;
template auto operator*
    <unsigned int>(const unsigned int lhs, const Amount& rhs) noexcept(false)
        -> Amount;
template auto operator*
    <unsigned long>(const unsigned long lhs, const Amount& rhs) noexcept(false)
        -> Amount;
template auto operator*<unsigned long long>(
    const unsigned long long lhs,
    const Amount& rhs) noexcept(false) -> Amount;

}  // namespace opentxs

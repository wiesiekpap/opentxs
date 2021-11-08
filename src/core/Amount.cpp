// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "opentxs/core/Amount.hpp"  // IWYU pragma: associated

#include <boost/cstdint.hpp>
#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>

#include "core/Amount.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/util/Log.hpp"

namespace be = boost::endian;

namespace opentxs
{

auto Amount::operator<(const Amount& rhs) const noexcept -> bool
{
    return imp_->amount_ < rhs.imp_->amount_;
}

auto Amount::operator<(const int rhs) const noexcept -> bool
{
    return imp_->amount_ < rhs;
}

auto Amount::operator<(const long int rhs) const noexcept -> bool
{
    return imp_->amount_ < rhs;
}

auto Amount::operator<(const long long int rhs) const noexcept -> bool
{
    return imp_->amount_ < rhs;
}

auto Amount::operator<(const unsigned long long int rhs) const noexcept -> bool
{
    return imp_->amount_ < rhs;
}

auto Amount::operator>(const Amount& rhs) const noexcept -> bool
{
    return imp_->amount_ > rhs.imp_->amount_;
}

auto Amount::operator>(const int rhs) const noexcept -> bool
{
    return imp_->amount_ > rhs;
}

auto Amount::operator>(const long int rhs) const noexcept -> bool
{
    return imp_->amount_ > rhs;
}

auto Amount::operator>(const long long int rhs) const noexcept -> bool
{
    return imp_->amount_ > rhs;
}

auto Amount::operator>(const unsigned long long int rhs) const noexcept -> bool
{
    return imp_->amount_ > rhs;
}

auto Amount::operator==(const Amount& rhs) const noexcept -> bool
{
    return imp_->amount_ == rhs.imp_->amount_;
}

auto Amount::operator==(const int rhs) const noexcept -> bool
{
    return imp_->amount_ == rhs;
}

auto Amount::operator==(const long int rhs) const noexcept -> bool
{
    return imp_->amount_ == rhs;
}

auto Amount::operator==(const long long int rhs) const noexcept -> bool
{
    return imp_->amount_ == rhs;
}

auto Amount::operator==(const unsigned int rhs) const noexcept -> bool
{
    return imp_->amount_ == rhs;
}

auto Amount::operator==(const unsigned long int rhs) const noexcept -> bool
{
    return imp_->amount_ == rhs;
}

auto Amount::operator==(const unsigned long long int rhs) const noexcept -> bool
{
    return imp_->amount_ == rhs;
}

auto Amount::operator!=(const Amount& rhs) const noexcept -> bool
{
    return imp_->amount_ != rhs.imp_->amount_;
}

auto Amount::operator!=(const int rhs) const noexcept -> bool
{
    return imp_->amount_ != rhs;
}

auto Amount::operator!=(const long int rhs) const noexcept -> bool
{
    return imp_->amount_ != rhs;
}

auto Amount::operator!=(const long long int rhs) const noexcept -> bool
{
    return imp_->amount_ != rhs;
}

auto Amount::operator!=(const unsigned long long int rhs) const noexcept -> bool
{
    return imp_->amount_ != rhs;
}

auto Amount::operator<=(const Amount& rhs) const noexcept -> bool
{
    return imp_->amount_ <= rhs.imp_->amount_;
}

auto Amount::operator<=(const int rhs) const noexcept -> bool
{
    return imp_->amount_ <= rhs;
}

auto Amount::operator<=(const long long int rhs) const noexcept -> bool
{
    return imp_->amount_ <= rhs;
}

auto Amount::operator<=(const unsigned int rhs) const noexcept -> bool
{
    return imp_->amount_ <= rhs;
}

auto Amount::operator<=(const unsigned long int rhs) const noexcept -> bool
{
    return imp_->amount_ <= rhs;
}

auto Amount::operator<=(const unsigned long long int rhs) const noexcept -> bool
{
    return imp_->amount_ <= rhs;
}

auto Amount::operator>=(const Amount& rhs) const noexcept -> bool
{
    return imp_->amount_ >= rhs.imp_->amount_;
}

auto Amount::operator>=(const long int rhs) const noexcept -> bool
{
    return imp_->amount_ >= rhs;
}

auto Amount::operator>=(const long long int rhs) const noexcept -> bool
{
    return imp_->amount_ >= rhs;
}

auto Amount::operator>=(const unsigned long long int rhs) const noexcept -> bool
{
    return imp_->amount_ >= rhs;
}

auto Amount::operator+(const Amount& rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ + rhs.imp_->amount_;
    return Amount(Imp(total));
}

auto Amount::operator+(const long int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ + rhs;
    return Amount(Imp(total));
}

auto Amount::operator+(const long long int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ + rhs;
    return Amount(Imp(total));
}

auto Amount::operator+(const unsigned int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ + rhs;
    return Amount(Imp(total));
}

auto Amount::operator+(const unsigned long int rhs) const noexcept(false)
    -> Amount
{
    const auto total = imp_->amount_ + rhs;
    return Amount(Imp(total));
}

auto Amount::operator+(const unsigned long long int rhs) const noexcept(false)
    -> Amount
{
    const auto total = imp_->amount_ + rhs;
    return Amount(Imp(total));
}

auto Amount::operator-(const Amount& rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ - rhs.imp_->amount_;
    return Amount(Imp(total));
}

auto Amount::operator-(const long int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ - rhs;
    return Amount(Imp(total));
}

auto Amount::operator-(const long long int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ - rhs;
    return Amount(Imp(total));
}

auto Amount::operator-(const unsigned long int rhs) const noexcept(false)
    -> Amount
{
    const auto total = imp_->amount_ - rhs;
    return Amount(Imp(total));
}

auto Amount::operator-(const unsigned long long int rhs) const noexcept(false)
    -> Amount
{
    const auto total = imp_->amount_ - rhs;
    return Amount(Imp(total));
}

auto Amount::operator*(const Amount& rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ * rhs.imp_->amount_;
    return Amount(Imp(total));
}

auto Amount::operator*(const int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ * rhs;
    return Amount(Imp(total));
}

auto Amount::operator*(const long int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ * rhs;
    return Amount(Imp(total));
}

auto Amount::operator*(const long long int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ * rhs;
    return Amount(Imp(total));
}

auto Amount::operator*(const unsigned long long int rhs) const noexcept(false)
    -> Amount
{
    const auto total = imp_->amount_ * rhs;
    return Amount(Imp(total));
}

auto Amount::operator/(const Amount& rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ / rhs.imp_->amount_;
    return Amount(Imp(total));
}

auto Amount::operator/(const int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ / rhs;
    return Amount(Imp(total));
}

auto Amount::operator/(const long long int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ / rhs;
    return Amount(Imp(total));
}

auto Amount::operator/(const unsigned long long int rhs) const noexcept(false)
    -> Amount
{
    const auto total = imp_->amount_ / rhs;
    return Amount(Imp(total));
}

auto Amount::operator%(const Amount& rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ % rhs.imp_->amount_;
    return Amount(Imp(total));
}

auto Amount::operator%(const int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ % rhs;
    return Amount(Imp(total));
}

auto Amount::operator%(const long long int rhs) const noexcept(false) -> Amount
{
    const auto total = imp_->amount_ % rhs;
    return Amount(Imp(total));
}

auto Amount::operator%(const unsigned long long int rhs) const noexcept(false)
    -> Amount
{
    const auto total = imp_->amount_ % rhs;
    return Amount(Imp(total));
}

auto Amount::operator*=(const Amount& amount) noexcept(false) -> Amount&
{
    imp_->amount_ *= amount.imp_->amount_;
    return *this;
}

auto Amount::operator+=(const Amount& amount) noexcept(false) -> Amount&
{
    imp_->amount_ += amount.imp_->amount_;
    return *this;
}

auto Amount::operator+=(const unsigned long int amount) noexcept(false)
    -> Amount&
{
    imp_->amount_ += amount;
    return *this;
}

auto Amount::operator-=(const Amount& amount) noexcept(false) -> Amount&
{
    imp_->amount_ -= amount.imp_->amount_;
    return *this;
}

auto Amount::operator-=(const unsigned long int amount) noexcept(false)
    -> Amount&
{
    imp_->amount_ -= amount;
    return *this;
}

auto Amount::operator-() -> Amount
{
    auto amount = -imp_->amount_;
    return Amount(Imp(amount));
}

auto Amount::str() const -> std::string { return imp_->amount_.str(); }

auto Amount::SerializeBitcoin(const AllocateOutput dest) const noexcept -> bool
{
    if (imp_->amount_ < 0 ||
        imp_->amount_ > std::numeric_limits<std::int64_t>::max())
        return false;

    auto amount = std::int64_t{};
    try {
        amount = imp_->amount_.convert_to<std::int64_t>();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS(__func__))("Error serializing amount: ")(
            e.what())
            .Flush();
        return false;
    }
    const auto buffer = be::little_int64_buf_t(amount);

    const auto view =
        ReadView(reinterpret_cast<const char*>(&buffer), sizeof(buffer));

    copy(view, dest);

    return true;
}

auto Amount::SerializeBitcoinSize() noexcept -> std::size_t
{
    return sizeof(be::little_int64_buf_t);
}

auto Amount::Internal() const noexcept -> Imp&
{
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
Amount::Amount(std::string_view str) noexcept(false)
    : imp_(std::make_unique<Imp>(str).release())
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
    imp_->amount_ = amount.imp_->amount_;
    return *this;
}

auto Amount::operator=(Amount&& amount) -> Amount&
{
    if (nullptr != imp_) { delete imp_; }
    imp_ = amount.imp_;
    amount.imp_ = nullptr;
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
    return lhs < rhs.imp_->amount_;
}
auto operator<(const long int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs < rhs.imp_->amount_;
}
auto operator<(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs < rhs.imp_->amount_;
}
auto operator<(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    return lhs < rhs.imp_->amount_;
}

auto operator>(const int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs > rhs.imp_->amount_;
}
auto operator>(const long int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs > rhs.imp_->amount_;
}
auto operator>(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs > rhs.imp_->amount_;
}
auto operator>(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    return lhs > rhs.imp_->amount_;
}

auto operator==(const int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs == rhs.imp_->amount_;
}
auto operator==(const long int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs == rhs.imp_->amount_;
}
auto operator==(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs == rhs.imp_->amount_;
}
auto operator==(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    return lhs == rhs.imp_->amount_;
}

auto operator!=(const int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs != rhs.imp_->amount_;
}
auto operator!=(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs != rhs.imp_->amount_;
}
auto operator!=(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    return lhs != rhs.imp_->amount_;
}

auto operator<=(const long int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs <= rhs.imp_->amount_;
}
auto operator<=(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs <= rhs.imp_->amount_;
}
auto operator<=(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    return lhs <= rhs.imp_->amount_;
}

auto operator>=(const int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs >= rhs.imp_->amount_;
}
auto operator>=(const long long int lhs, const Amount& rhs) noexcept -> bool
{
    return lhs >= rhs.imp_->amount_;
}
auto operator>=(const unsigned long long int lhs, const Amount& rhs) noexcept
    -> bool
{
    return lhs >= rhs.imp_->amount_;
}

auto operator+(const long int lhs, const Amount& rhs) noexcept(false) -> Amount
{
    const auto total = lhs + rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}
auto operator+(const long long int lhs, const Amount& rhs) noexcept(false)
    -> Amount
{
    const auto total = lhs + rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}
auto operator+(const unsigned long int lhs, const Amount& rhs) noexcept(false)
    -> Amount
{
    const auto total = lhs + rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}

auto operator-(const long int lhs, const Amount& rhs) noexcept(false) -> Amount
{
    const auto total = lhs - rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}
auto operator-(const long long int lhs, const Amount& rhs) noexcept(false)
    -> Amount
{
    const auto total = lhs - rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}

auto operator*(const int lhs, const Amount& rhs) noexcept(false) -> Amount
{
    const auto total = lhs * rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}
auto operator*(const long long int lhs, const Amount& rhs) noexcept(false)
    -> Amount
{
    const auto total = lhs * rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}
auto operator*(const unsigned int lhs, const Amount& rhs) noexcept(false)
    -> Amount
{
    const auto total = lhs * rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}
auto operator*(const unsigned long int lhs, const Amount& rhs) noexcept(false)
    -> Amount
{
    const auto total = lhs * rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}
auto operator*(const unsigned long long int lhs, const Amount& rhs) noexcept(
    false) -> Amount
{
    const auto total = lhs * rhs.imp_->amount_;
    return Amount(Amount::Imp(total));
}

}  // namespace opentxs

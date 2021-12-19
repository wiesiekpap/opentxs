// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/cstdint.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <exception>
#include <limits>
#include <string>
#include <string_view>

#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"

namespace be = boost::endian;
namespace bmp = boost::multiprecision;

namespace opentxs
{
struct Amount::Imp {
    using Backend = bmp::number<bmp::cpp_int_backend<
        128,
        320,
        bmp::signed_magnitude,
        bmp::checked,
        void>>;

    Imp() noexcept
        : amount_{}
    {
    }
    Imp(const Backend& left_shifted_amount) noexcept
        : amount_{left_shifted_amount}
    {
    }
    Imp(int amount) noexcept
        : amount_{shift_left(amount)}
    {
    }
    Imp(long int amount) noexcept
        : amount_{shift_left(amount)}
    {
    }
    Imp(long long int amount) noexcept
        : amount_{shift_left(amount)}
    {
    }
    Imp(unsigned int amount) noexcept
        : amount_{shift_left(amount)}
    {
    }
    Imp(unsigned long int amount) noexcept
        : amount_{shift_left(amount)}
    {
    }
    Imp(unsigned long long int amount) noexcept
        : amount_{shift_left(amount)}
    {
    }
    Imp(std::string_view str, bool normalize = false) noexcept(false)
        : amount_{Backend{str}}
    {
        if (true == normalize) {
            if (amount_ < 0) {
                amount_ = -(-amount_ << 64);
            } else {
                amount_ <<= 64;
            }
        }
    }

    auto operator=(const Imp& imp) -> Imp&
    {
        amount_ = imp.amount_;
        return *this;
    }

    auto operator<(const Imp& rhs) { return amount_ < rhs.amount_; }

    template <typename T>
    auto operator<(const T rhs)
    {
        return amount_ < shift_left(rhs);
    }

    auto operator>(const Imp& rhs) { return amount_ > rhs.amount_; }

    template <typename T>
    auto operator>(const T rhs)
    {
        return amount_ > shift_left(rhs);
    }

    auto operator==(const Imp& rhs) { return amount_ == rhs.amount_; }

    template <typename T>
    auto operator==(const T rhs)
    {
        return amount_ == shift_left(rhs);
    }

    auto operator!=(const Imp& rhs) { return amount_ != rhs.amount_; }

    template <typename T>
    auto operator!=(const T rhs)
    {
        return amount_ != shift_left(rhs);
    }

    auto operator<=(const Imp& rhs) { return amount_ <= rhs.amount_; }

    template <typename T>
    auto operator<=(const T rhs)
    {
        return amount_ <= shift_left(rhs);
    }

    auto operator>=(const Imp& rhs) { return amount_ >= rhs.amount_; }

    template <typename T>
    auto operator>=(const T rhs)
    {
        return amount_ >= shift_left(rhs);
    }

    auto operator+(const Imp& rhs) const noexcept(false) -> Imp
    {
        return amount_ + rhs.amount_;
    }

    auto operator-(const Imp& rhs) const noexcept(false) -> Imp
    {
        return amount_ - rhs.amount_;
    }

    auto operator*(const Imp& rhs) const noexcept(false) -> Imp
    {
        const auto total = amount_ * rhs.amount_;
        auto imp = Imp{total};
        return imp.shift_right();
    }

    template <typename T>
    auto operator*(const T rhs) const noexcept(false) -> Imp
    {
        return amount_ * rhs;
    }

    auto operator/(const Imp& rhs) const noexcept(false) -> Imp
    {
        const auto total = amount_ / rhs.amount_;
        return Imp::shift_left(total);
    }

    template <typename T>
    auto operator/(const T rhs) const noexcept(false) -> Imp
    {
        return amount_ / rhs;
    }

    auto operator%(const Imp& rhs) const noexcept(false) -> Imp
    {
        return amount_ % rhs.amount_;
    }

    template <typename T>
    auto operator%(const T rhs) const noexcept(false) -> Imp
    {
        return amount_ % shift_left(rhs);
    }

    auto operator*=(const Imp& amount) noexcept(false) -> Imp&
    {
        auto total = amount_ * amount.amount_;
        amount_ = Imp(total).shift_right();
        return *this;
    }

    auto operator+=(const Imp& amount) noexcept(false) -> Imp&
    {
        amount_ += amount.amount_;
        return *this;
    }

    auto operator-=(const Imp& amount) noexcept(false) -> Imp&
    {
        amount_ -= amount.amount_;
        return *this;
    }

    auto operator-() -> Imp { return -amount_; }

    auto Serialize(const AllocateOutput dest) const noexcept -> bool
    {
        auto amount = std::string{};

        try {
            amount = amount_.str();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())("Error serializing amount: ")(
                e.what())
                .Flush();
            return false;
        }

        return copy(amount, dest);
    }

    auto SerializeBitcoin(const AllocateOutput dest) const noexcept -> bool
    {
        const auto backend = shift_right();
        if (backend < 0 || backend > std::numeric_limits<std::int64_t>::max())
            return false;

        auto amount = std::int64_t{};
        try {
            amount = backend.convert_to<std::int64_t>();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())("Error serializing bitcoin amount: ")(
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

    template <typename T>
    auto extract_int() const noexcept -> T
    {
        try {
            return shift_right().convert_to<T>();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())("Error converting Amount to int: ")(
                e.what())
                .Flush();
            return {};
        }
    }

    template <typename T>
    auto extract_float() const noexcept -> T
    {
        try {
            return amount_.convert_to<T>() / shift_left(1).convert_to<T>();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())("Error converting Amount to float: ")(
                e.what())
                .Flush();
            return {};
        }
    }

    template <typename T>
    static auto shift_left(const T& amount) -> Backend
    {
        if (amount < 0) {
            auto tmp = -Backend{amount};
            tmp <<= 64;
            return -tmp;
        } else {
            return Backend{amount} << 64;
        }
    }

    auto shift_right() const -> Backend
    {
        if (amount_ < 0) {
            auto tmp = -amount_;
            tmp >>= 64;
            return -tmp;
        } else {
            return amount_ >> 64;
        }
    }

    Imp(const Imp& rhs) noexcept = default;
    Imp(Imp&& rhs) noexcept = delete;
    auto operator=(Imp&& rhs) -> Imp& = delete;

    Backend amount_;
};
}  // namespace opentxs

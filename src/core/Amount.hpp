// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <string_view>

#include "opentxs/core/Amount.hpp"

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
    Imp(const Backend& amount) noexcept
        : amount_(amount)
    {
    }
    Imp(int amount) noexcept
        : amount_(amount)
    {
    }
    Imp(long int amount) noexcept
        : amount_(amount)
    {
    }
    Imp(long long int amount) noexcept
        : amount_(amount)
    {
    }
    Imp(unsigned int amount) noexcept
        : amount_(amount)
    {
    }
    Imp(unsigned long int amount) noexcept
        : amount_(amount)
    {
    }
    Imp(unsigned long long int amount) noexcept
        : amount_(amount)
    {
    }
    Imp(std::string_view str) noexcept(false)
        : amount_(str)
    {
    }

    Imp(const Imp& rhs) noexcept = default;
    Imp(Imp&& rhs) noexcept = delete;
    auto operator=(const Imp& rhs) -> Imp& = delete;
    auto operator=(Imp&& rhs) -> Imp& = delete;

    Backend amount_;
};
}  // namespace opentxs

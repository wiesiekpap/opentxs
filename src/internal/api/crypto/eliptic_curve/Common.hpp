//
// Created by PetterFi on 09.08.22.
//
#pragma once

#include <cstdint>
#include <string_view>

#include <boost/multiprecision/number.hpp>

namespace opentxs::api::crypto::eliptic_curve
{

template <typename T>
std::string to_hex(boost::multiprecision::number<T> const& value)
{
    return value.str(0, std::ios_base::hex);
}

template <typename T>
bool is_even(T value)
{
    return value % 2 == 0;
}

std::string_view get_pubkey_prefix(
    std::string_view pubkey_sv,
    std::size_t const prefix_size);
}  // namespace opentxs::api::crypto::eliptic_curve

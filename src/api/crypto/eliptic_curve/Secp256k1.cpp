//
// Created by PetterFi on 09.08.22.
//
#include <string_view>

#include <boost/multiprecision/cpp_int.hpp>

#include "internal/api/crypto/eliptic_curve/Common.hpp"
#include "internal/api/crypto/eliptic_curve/Secp256k1.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/StringJoiner.hpp"
#include "opentxs/util/Log.hpp"

namespace bmp = boost::multiprecision;

namespace opentxs::api::crypto::eliptic_curve::secp256k1
{
constexpr auto PUBKEY_PREFIX_LENGTH{2U};
constexpr auto COMPRESSED_PUBKEY_LENGTH{64U};

constexpr std::string_view COMPRESSED_PUBKEY_EVEN_PREFIX_V{"02"};
constexpr std::string_view COMPRESSED_PUBKEY_ODD_PREFIX_V{"03"};

bmp::uint1024_t const p{
    "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f"};
bmp::uint1024_t const a{
    "0x0000000000000000000000000000000000000000000000000000000000000000"};
bmp::uint1024_t const b{
    "0x0000000000000000000000000000000000000000000000000000000000000007"};

bool is_pubkey_starts_from_even_prefix(std::string_view key_prefix)
{
    return key_prefix == COMPRESSED_PUBKEY_EVEN_PREFIX_V;
}

bool is_pubkey_starts_from_odd_prefix(std::string_view key_prefix)
{
    return key_prefix == COMPRESSED_PUBKEY_ODD_PREFIX_V;
}

bool is_compressed_pubkey_correct_size(std::size_t const pubkey_length)
{
    return COMPRESSED_PUBKEY_LENGTH == pubkey_length;
}

bool is_pubkey_compressed(std::string_view key_prefix)
{
    return is_pubkey_starts_from_even_prefix(key_prefix) or
           is_pubkey_starts_from_odd_prefix(key_prefix);
}

std::string get_uncompressed_pubkey(std::string_view compressed_pubkey_sv)
{
    std::string uncompressed_key;
    auto const pubkey_length =
        compressed_pubkey_sv.size() - PUBKEY_PREFIX_LENGTH;
    auto extracted_compressed_key =
        compressed_pubkey_sv.substr(PUBKEY_PREFIX_LENGTH);

    auto const key_prefix =
        get_pubkey_prefix(compressed_pubkey_sv, PUBKEY_PREFIX_LENGTH);

    if (!is_pubkey_compressed(key_prefix))
        opentxs::LogError()(__func__)(": ")("Compressed pubkey ")(
            compressed_pubkey_sv)(" is not valid - incorrect prefix")
            .Flush();
    if (!is_compressed_pubkey_correct_size(pubkey_length))
        opentxs::LogError()(__func__)(": ")("Compressed pubkey ")(
            compressed_pubkey_sv)(" has not correct size ")(
            pubkey_length)(" instead of ")(COMPRESSED_PUBKEY_LENGTH)
            .Flush();

    bmp::uint1024_t const x{opentxs::join("0x", extracted_compressed_key)};
    bmp::uint1024_t const rhs = (bmp::powm(x, 3, p) + (a * x) + b) % p;
    bmp::uint1024_t const y = bmp::powm(rhs, (p + 1) / 4, p);

    bmp::uint1024_t compressed_y = p - y;

    opentxs::LogDebug()(__func__)(": ")("lhs ")(
        bmp::powm(compressed_y, 2, p).str())(" = rhs ")(rhs.str())
        .Flush();
    opentxs::LogDebug()(__func__)(": ")("lhs ")(bmp::powm(y, 2, p).str())(
        " = rhs ")(rhs.str())
        .Flush();

    opentxs::LogDebug()(__func__)(": ")("x = ")(
        add_leading_zeros(to_hex(x), 64))
        .Flush();
    opentxs::LogDebug()(__func__)(": ")("y = ")(
        add_leading_zeros(to_hex(y), 64))
        .Flush();
    opentxs::LogDebug()(__func__)(": ")("rhs = ")(
        add_leading_zeros(to_hex(rhs), 64))
        .Flush();
    opentxs::LogDebug()(__func__)(": ")("compressed_y= ")(
        add_leading_zeros(to_hex(compressed_y), 64))
        .Flush();
    opentxs::LogDebug()(__func__)(": ")("even = ")(is_even(compressed_y))
        .Flush();

    if (!(bmp::powm(compressed_y, 2, p) == rhs))
        opentxs::LogError()(__func__)(": ")(
            "The equation using compressed y is not correct")
            .Flush();
    if (!(bmp::powm(y, 2, p) == rhs))
        opentxs::LogError()(__func__)(": ")(
            "The equation using y is not correct")
            .Flush();
    if (is_pubkey_starts_from_even_prefix(key_prefix)) {
        if (is_even(compressed_y))
            uncompressed_key = opentxs::join(
                "04",
                extracted_compressed_key,
                add_leading_zeros(to_hex(compressed_y), 64));
        else if (is_even(y))
            uncompressed_key = opentxs::join(
                "04",
                extracted_compressed_key,
                add_leading_zeros(to_hex(y), 64));
        else
            opentxs::LogError()(__func__)(": ")(
                "Uncompressed key cannot be calculated ")(
                "for pubkey starting from even prefix because ")(
                "is_even(compressed_y)= ")(is_even(compressed_y))(
                "is_even(y)= ")(is_even(y))
                .Flush();
    } else if (is_pubkey_starts_from_odd_prefix(key_prefix)) {
        if (is_odd(compressed_y))
            uncompressed_key = opentxs::join(
                "04",
                extracted_compressed_key,
                add_leading_zeros(to_hex(compressed_y), 64));
        else if (is_odd(y)) {
            uncompressed_key = opentxs::join(
                "04",
                extracted_compressed_key,
                add_leading_zeros(to_hex(y), 64));
        } else
            opentxs::LogError()(__func__)(": ")(
                "Uncompressed key cannot be calculated ")(
                "for pubkey starting from odd prefix because ")(
                "is_odd(compressed_y)= ")(is_odd(compressed_y))("is_odd(y)= ")(
                is_odd(y))
                .Flush();
    } else
        opentxs::LogError()(__func__)(": ")("Pubkey prefix is not valid ")(
            "pubkey doesn't starts from even or odd prefix")
            .Flush();
    return uncompressed_key;
}
}  // namespace
   // opentxs::api::crypto::eliptic_curve::secp256k1

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "api/crypto/Hash.hpp"  // IWYU pragma: associated

#include <boost/algorithm/hex.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>

#include "internal/api/crypto/eliptic_curve/Secp256k1.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/crypto/library/Pbkdf2.hpp"
#include "internal/crypto/library/Ripemd160.hpp"
#include "internal/crypto/library/Scrypt.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "smhasher/src/MurmurHash3.h"

namespace bmp = boost::multiprecision;

namespace opentxs::factory
{
auto Hash(
    const api::crypto::Encode& encode,
    const crypto::HashingProvider& sha,
    const crypto::HashingProvider& blake,
    const crypto::HashingProvider& keccak,
    const crypto::Pbkdf2& pbkdf2,
    const crypto::Ripemd160& ripe,
    const crypto::Scrypt& scrypt) noexcept -> std::unique_ptr<api::crypto::Hash>
{
    using ReturnType = api::crypto::imp::Hash;

    return std::make_unique<ReturnType>(
        encode, sha, blake, keccak, pbkdf2, ripe, scrypt);
}
}  // namespace opentxs::factory

namespace opentxs::api::crypto::imp
{
using Provider = opentxs::crypto::HashingProvider;

Hash::Hash(
    const api::crypto::Encode& encode,
    const Provider& sha,
    const Provider& blake,
    const Provider& keccak,
    const opentxs::crypto::Pbkdf2& pbkdf2,
    const opentxs::crypto::Ripemd160& ripe,
    const opentxs::crypto::Scrypt& scrypt) noexcept
    : encode_(encode)
    , sha_(sha)
    , blake_(blake)
    , keccak_(keccak)
    , pbkdf2_(pbkdf2)
    , ripe_(ripe)
    , scrypt_(scrypt)
{
}

auto Hash::bitcoin_hash_160(
    const ReadView data,
    const AllocateOutput destination) const noexcept -> bool
{
    try {
        auto temp = Space{};
        const auto rc =
            Digest(opentxs::crypto::HashType::Sha256, data, writer(temp));

        if (false == rc) {

            throw std::runtime_error{"failed to calculate intermediate hash"};
        }

        return Digest(
            opentxs::crypto::HashType::Ripemd160, reader(temp), destination);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Hash::ethereum_address(
    const ReadView keccakData,
    const AllocateOutput destination) const -> bool
{
    try {
        const int eth_address_length = 20;
        auto keccak_data_bytes =
            reinterpret_cast<const uint8_t*>(keccakData.data());
        const uint8_t* last_20_data_bytes =
            keccak_data_bytes + keccakData.size() - eth_address_length;

        auto address = destination(eth_address_length);

        if (false == address.valid(eth_address_length)) {
            LogError()(__func__)(": Failed to allocate space for an address")
                .Flush();

            return false;
        }

        address.as<unsigned char>();
        std::memcpy(address, last_20_data_bytes, eth_address_length);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Hash::ethereum_hash(const ReadView data, const AllocateOutput destination)
    const noexcept -> bool
{
    try {
        auto uncompressed_pubkey_temp = Space{};
        const auto success =
            uncompress_pubkey(data, writer(uncompressed_pubkey_temp));
        if (false == success) {

            throw std::runtime_error{"failed to calculate uncompressed pubkey"};
        }
        ReadView uncompressed_pubkey_without_prefix_readview(
            reinterpret_cast<const char*>(uncompressed_pubkey_temp.data()),
            uncompressed_pubkey_temp.size());

        auto keccak_temp =
            space(Provider::HashSize(opentxs::crypto::HashType::Keccak256));

        const auto rc = Digest(
            opentxs::crypto::HashType::Keccak256,
            uncompressed_pubkey_without_prefix_readview,
            writer(keccak_temp));

        if (false == rc) {

            throw std::runtime_error{
                "failed to calculate intermediate keccak hash"};
        }

        return ethereum_address(reader(keccak_temp), destination);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Hash::uncompress_pubkey(const ReadView data, const AllocateOutput output)
    const noexcept -> bool
{
    try {
        if (false == output.operator bool()) {
            throw std::runtime_error{"invalid output"};
        }

        const auto compressed_pubkey_size = 33;
        if (data.size() > compressed_pubkey_size) {
            throw std::runtime_error{"input too large"};
        }

        const auto uncompressed_without_prefix_size = 64;
        auto buf = output(uncompressed_without_prefix_size);
        if (false == buf.valid(uncompressed_without_prefix_size)) {
            throw std::runtime_error{"failed to allocate space for output"};
        }

        auto hex_data = to_hex(
            reinterpret_cast<const std::byte*>(data.data()), data.size());
        ReadView hex_data_sv(hex_data.data(), hex_data.size());
        auto uncompressed_pubkey_str =
            opentxs::api::crypto::eliptic_curve::secp256k1::get_uncompressed_pubkey(hex_data_sv);

        static const int eth_uncompressed_public_key_prefix_length = 2;
        // convert char to hex - treat 2 char sign as a byte
        std::vector<unsigned char> uncompressed_pubkey_bytes;
        boost::algorithm::unhex(
            uncompressed_pubkey_str.substr(
                eth_uncompressed_public_key_prefix_length),
            std::back_inserter(uncompressed_pubkey_bytes));
        std::memcpy(
            buf.as<unsigned char>(),
            uncompressed_pubkey_bytes.data(),
            uncompressed_pubkey_bytes.size());
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }

    return true;
}

auto Hash::Digest(
    const opentxs::crypto::HashType type,
    const ReadView data,
    const AllocateOutput destination) const noexcept -> bool
{
    switch (type) {
        case opentxs::crypto::HashType::Sha1:
        case opentxs::crypto::HashType::Sha256:
        case opentxs::crypto::HashType::Sha512: {

            return sha_.Digest(type, data, destination);
        }
        case opentxs::crypto::HashType::Blake2b160:
        case opentxs::crypto::HashType::Blake2b256:
        case opentxs::crypto::HashType::Blake2b512: {

            return blake_.Digest(type, data, destination);
        }
        case opentxs::crypto::HashType::Ripemd160: {

            return ripe_.RIPEMD160(data, destination);
        }
        case opentxs::crypto::HashType::Sha256D: {

            return sha_256_double(data, destination);
        }
        case opentxs::crypto::HashType::Sha256DC: {

            return sha_256_double_checksum(data, destination);
        }
        case opentxs::crypto::HashType::Bitcoin: {

            return bitcoin_hash_160(data, destination);
        }
        case opentxs::crypto::HashType::Ethereum: {
            return ethereum_hash(data, destination);
        }
        case opentxs::crypto::HashType::Keccak256: {
            return keccak_.Digest(type, data, destination);
        }
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported hash type.").Flush();

            return false;
        }
    }
}

auto Hash::Digest(
    const opentxs::crypto::HashType type,
    const opentxs::network::zeromq::Frame& data,
    const AllocateOutput destination) const noexcept -> bool
{
    return Digest(type, data.Bytes(), destination);
}

auto Hash::Digest(
    const std::uint32_t hash,
    const ReadView data,
    const AllocateOutput destination) const noexcept -> bool
{
    const auto type = static_cast<opentxs::crypto::HashType>(hash);
    auto temp =
        Data::Factory();  // TODO IdentifierEncode should accept ReadView

    try {
        if (false == Digest(type, data, temp->WriteInto())) {

            throw std::runtime_error{"failed to calculate hash"};
        }

        const auto encoded = encode_.IdentifierEncode(temp);
        auto output = destination(encoded.size());

        if (false == output.valid(encoded.size())) {
            throw std::runtime_error{"unable to allocate encoded output space"};
        }

        std::memcpy(output, encoded.data(), output);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Hash::HMAC(
    const opentxs::crypto::HashType type,
    const ReadView key,
    const ReadView data,
    const AllocateOutput output) const noexcept -> bool
{
    switch (type) {
        case opentxs::crypto::HashType::Sha256:
        case opentxs::crypto::HashType::Sha512: {

            return sha_.HMAC(type, key, data, output);
        }
        case opentxs::crypto::HashType::Blake2b160:
        case opentxs::crypto::HashType::Blake2b256:
        case opentxs::crypto::HashType::Blake2b512:
        case opentxs::crypto::HashType::SipHash24: {

            return blake_.HMAC(type, key, data, output);
        }

        case opentxs::crypto::HashType::Keccak256:
        case opentxs::crypto::HashType::Ethereum:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported hash type.").Flush();

            return false;
        }
    }
}

auto Hash::MurmurHash3_32(
    const std::uint32_t& key,
    const Data& data,
    std::uint32_t& output) const noexcept -> void
{
    const auto size = data.size();

    OT_ASSERT(size <= std::numeric_limits<int>::max());

    MurmurHash3_x86_32(
        data.data(), static_cast<int>(data.size()), key, &output);
}

auto Hash::PKCS5_PBKDF2_HMAC(
    const Data& input,
    const Data& salt,
    const std::size_t iterations,
    const opentxs::crypto::HashType type,
    const std::size_t bytes,
    Data& output) const noexcept -> bool
{
    output.SetSize(bytes);

    return pbkdf2_.PKCS5_PBKDF2_HMAC(
        input.data(),
        input.size(),
        salt.data(),
        salt.size(),
        iterations,
        type,
        bytes,
        output.data());
}

auto Hash::PKCS5_PBKDF2_HMAC(
    const Secret& input,
    const Data& salt,
    const std::size_t iterations,
    const opentxs::crypto::HashType type,
    const std::size_t bytes,
    Data& output) const noexcept -> bool
{
    output.SetSize(bytes);
    const auto data = input.Bytes();

    return pbkdf2_.PKCS5_PBKDF2_HMAC(
        data.data(),
        data.size(),
        salt.data(),
        salt.size(),
        iterations,
        type,
        bytes,
        output.data());
}

auto Hash::PKCS5_PBKDF2_HMAC(
    const UnallocatedCString& input,
    const Data& salt,
    const std::size_t iterations,
    const opentxs::crypto::HashType type,
    const std::size_t bytes,
    Data& output) const noexcept -> bool
{
    output.SetSize(bytes);

    return pbkdf2_.PKCS5_PBKDF2_HMAC(
        input.data(),
        input.size(),
        salt.data(),
        salt.size(),
        iterations,
        type,
        bytes,
        output.data());
}

auto Hash::Scrypt(
    const ReadView input,
    const ReadView salt,
    const std::uint64_t N,
    const std::uint32_t r,
    const std::uint32_t p,
    const std::size_t bytes,
    AllocateOutput writer) const noexcept -> bool
{
    return scrypt_.Generate(input, salt, N, r, p, bytes, writer);
}

auto Hash::sha_256_double(const ReadView data, const AllocateOutput destination)
    const noexcept -> bool
{
    try {
        auto temp = Space{};
        const auto rc =
            Digest(opentxs::crypto::HashType::Sha256, data, writer(temp));

        if (false == rc) {

            throw std::runtime_error{"failed to calculate intermediate hash"};
        }

        return Digest(
            opentxs::crypto::HashType::Sha256, reader(temp), destination);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Hash::sha_256_double_checksum(
    const ReadView data,
    const AllocateOutput destination) const noexcept -> bool
{
    try {
        auto temp = Space{};
        const auto rc = sha_256_double(data, writer(temp));

        if (false == rc) {

            throw std::runtime_error{"failed to calculate intermediate hash"};
        }

        if (false == destination.operator bool()) {
            throw std::runtime_error{"invalid output"};
        }

        static constexpr auto size = 4_uz;
        auto buf = destination(size);

        if (false == buf.valid(size)) {
            throw std::runtime_error{"failed to allocate space for output"};
        }

        std::memcpy(buf.data(), temp.data(), size);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::api::crypto::imp

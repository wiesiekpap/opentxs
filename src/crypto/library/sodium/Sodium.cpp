// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "crypto/library/sodium/Sodium.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <SHA1/sha1.hpp>
#include <argon2.h>
#include <array>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "crypto/library/AsymmetricProvider.hpp"
#include "crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/library/Factory.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Primitives.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/SecretStyle.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/crypto/key/symmetric/Source.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/protobuf/Ciphertext.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#include "util/Sodium.hpp"

#define OT_METHOD "opentxs::crypto::implementation::Sodium::"

namespace opentxs::factory
{
auto Sodium(const api::Crypto& crypto) noexcept
    -> std::unique_ptr<crypto::Sodium>
{
    using ReturnType = crypto::implementation::Sodium;

    return std::make_unique<ReturnType>(crypto);
}
}  // namespace opentxs::factory

namespace opentxs::crypto::implementation
{
Sodium::Sodium(const api::Crypto& crypto) noexcept
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    : AsymmetricProvider()
    , EcdsaProvider(crypto)
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
{
    const auto result = ::sodium_init();

    OT_ASSERT(-1 != result);
}

auto Sodium::blank_private() noexcept -> ReadView
{
    static const auto blank = space(crypto_sign_SECRETKEYBYTES);

    return reader(blank);
}

auto Sodium::Decrypt(
    const proto::Ciphertext& ciphertext,
    const std::uint8_t* key,
    const std::size_t keySize,
    std::uint8_t* plaintext) const -> bool
{
    const auto& message = ciphertext.data();
    const auto& nonce = ciphertext.iv();
    const auto& mac = ciphertext.tag();
    const auto& mode = ciphertext.mode();

    if (KeySize(opentxs::crypto::key::internal::translate(mode)) != keySize) {
        LogOutput(OT_METHOD)(__func__)(": Incorrect key size.").Flush();

        return false;
    }

    if (IvSize(opentxs::crypto::key::internal::translate(mode)) !=
        nonce.size()) {
        LogOutput(OT_METHOD)(__func__)(": Incorrect nonce size.").Flush();

        return false;
    }

    switch (ciphertext.mode()) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return (
                0 == crypto_aead_chacha20poly1305_ietf_decrypt_detached(
                         plaintext,
                         nullptr,
                         reinterpret_cast<const unsigned char*>(message.data()),
                         message.size(),
                         reinterpret_cast<const unsigned char*>(mac.data()),
                         nullptr,
                         0,
                         reinterpret_cast<const unsigned char*>(nonce.data()),
                         key));
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unsupported encryption mode (")(
                mode)(").")
                .Flush();
        }
    }

    return false;
}

auto Sodium::Derive(
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* salt,
    const std::size_t saltSize,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const key::symmetric::Source type,
    std::uint8_t* output,
    std::size_t outputSize) const -> bool
{
    try {
        const auto minOps = [&] {
            switch (type) {
                case key::symmetric::Source::Argon2i: {

                    return crypto_pwhash_argon2i_OPSLIMIT_MIN;
                }
                case key::symmetric::Source::Argon2id: {

                    return crypto_pwhash_argon2id_OPSLIMIT_MIN;
                }
                default: {

                    throw std::runtime_error{"unsupported algorithm"};
                }
            }
        }();
        const auto requiredSize = SaltSize(type);

        if (requiredSize != saltSize) {
            const auto error = std::string{"Incorrect salt size ("} +
                               std::to_string(saltSize) + "). Required: (" +
                               std::to_string(requiredSize);

            throw std::runtime_error{error};
        }

        OT_ASSERT(nullptr != salt);

        if (outputSize < crypto_pwhash_BYTES_MIN) {
            throw std::runtime_error{"output too small"};
        }

        if (outputSize > crypto_pwhash_BYTES_MAX) {
            throw std::runtime_error{"output too large"};
        }

        if (inputSize > crypto_pwhash_PASSWD_MAX) {
            throw std::runtime_error{"input too large"};
        }

        if (operations < minOps) {
            throw std::runtime_error{"operations too low"};
        }

        if (operations > crypto_pwhash_OPSLIMIT_MAX) {
            throw std::runtime_error{"operations too high"};
        }

        if (difficulty < crypto_pwhash_MEMLIMIT_MIN) {
            throw std::runtime_error{"memory too low"};
        }

        if (difficulty > crypto_pwhash_MEMLIMIT_MAX) {
            throw std::runtime_error{"memory too high"};
        }

        if (parallel > ARGON2_MAX_THREADS) {
            throw std::runtime_error{"too many threads"};
        }

        static const auto blank = char{};
        const auto empty = ((nullptr == input) || (0u == inputSize));
        const auto* ptr = empty ? &blank : reinterpret_cast<const char*>(input);
        const auto effective = empty ? std::size_t{0u} : inputSize;

        if (1u < parallel) {
            const auto rc = [&] {
                switch (type) {
                    case key::symmetric::Source::Argon2i: {

                        return ::argon2i_hash_raw_fucklibsodium(
                            static_cast<std::uint32_t>(operations),
                            static_cast<std::uint32_t>(difficulty >> 10),
                            static_cast<std::uint32_t>(parallel),
                            ptr,
                            effective,
                            salt,
                            saltSize,
                            output,
                            outputSize);
                    }
                    case key::symmetric::Source::Argon2id: {

                        return ::argon2id_hash_raw_fucklibsodium(
                            static_cast<std::uint32_t>(operations),
                            static_cast<std::uint32_t>(difficulty >> 10),
                            static_cast<std::uint32_t>(parallel),
                            ptr,
                            effective,
                            salt,
                            saltSize,
                            output,
                            outputSize);
                    }
                    default: {

                        throw std::runtime_error{"unsupported algorithm"};
                    }
                }
            }();

            if (ARGON2_OK != rc) {
                throw std::runtime_error{
                    ::argon2_error_message_fucklibsodium(rc)};
            }
        } else {
            const auto rc = [&] {
                switch (type) {
                    case key::symmetric::Source::Argon2i: {

                        return ::crypto_pwhash(
                            output,
                            outputSize,
                            ptr,
                            effective,
                            salt,
                            operations,
                            difficulty,
                            crypto_pwhash_ALG_ARGON2I13);
                    }
                    case key::symmetric::Source::Argon2id: {

                        return ::crypto_pwhash(
                            output,
                            outputSize,
                            ptr,
                            effective,
                            salt,
                            operations,
                            difficulty,
                            crypto_pwhash_ALG_ARGON2ID13);
                    }
                    default: {

                        throw std::runtime_error{"unsupported algorithm"};
                    }
                }
            }();

            if (0 != rc) { throw std::runtime_error{"failed to derive hash"}; }
        }

        return true;
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return false;
    }
}

auto Sodium::Digest(
    const crypto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    std::uint8_t* output) const -> bool
{
    switch (hashType) {
        case (crypto::HashType::Blake2b160):
        case (crypto::HashType::Blake2b256):
        case (crypto::HashType::Blake2b512): {
            return (
                0 == ::crypto_generichash(
                         output,
                         HashingProvider::HashSize(hashType),
                         input,
                         inputSize,
                         nullptr,
                         0));
        }
        case (crypto::HashType::Sha256): {
            return (0 == ::crypto_hash_sha256(output, input, inputSize));
        }
        case (crypto::HashType::Sha512): {
            return (0 == ::crypto_hash_sha512(output, input, inputSize));
        }
        case (crypto::HashType::Sha1): {
            return sha1(input, inputSize, output);
        }
        default: {
        }
    }

    LogOutput(OT_METHOD)(__func__)(": Unsupported hash function.").Flush();

    return false;
}

auto Sodium::Encrypt(
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* key,
    const std::size_t keySize,
    proto::Ciphertext& ciphertext) const -> bool
{
    OT_ASSERT(nullptr != input);
    OT_ASSERT(nullptr != key);

    const auto& mode =
        opentxs::crypto::key::internal::translate(ciphertext.mode());
    const auto& nonce = ciphertext.iv();
    auto& tag = *ciphertext.mutable_tag();
    auto& output = *ciphertext.mutable_data();

    bool result = false;

    if (mode == opentxs::crypto::key::symmetric::Algorithm::Error) {
        LogOutput(OT_METHOD)(__func__)(": Incorrect mode.").Flush();

        return result;
    }

    if (KeySize(mode) != keySize) {
        LogOutput(OT_METHOD)(__func__)(": Incorrect key size.").Flush();

        return result;
    }

    if (IvSize(mode) != nonce.size()) {
        LogOutput(OT_METHOD)(__func__)(": Incorrect nonce size.").Flush();

        return result;
    }

    ciphertext.set_version(1);
    tag.resize(TagSize(mode), 0x0);
    output.resize(inputSize, 0x0);

    OT_ASSERT(false == nonce.empty());
    OT_ASSERT(false == tag.empty());

    switch (mode) {
        case (opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305): {
            return (
                0 == crypto_aead_chacha20poly1305_ietf_encrypt_detached(
                         reinterpret_cast<unsigned char*>(output.data()),
                         reinterpret_cast<unsigned char*>(tag.data()),
                         nullptr,
                         input,
                         inputSize,
                         nullptr,
                         0,
                         nullptr,
                         reinterpret_cast<const unsigned char*>(nonce.data()),
                         key));
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unsupported encryption mode (")(
                value(mode))(").")
                .Flush();
        }
    }

    return result;
}

auto Sodium::Generate(
    const ReadView input,
    const ReadView salt,
    const std::uint64_t N,
    const std::uint32_t r,
    const std::uint32_t p,
    const std::size_t bytes,
    AllocateOutput writer) const noexcept -> bool
{
    if (false == bool(writer)) {
        LogOutput(OT_METHOD)(__func__)(": Invalid writer").Flush();

        return false;
    }

    if (bytes < crypto_pwhash_scryptsalsa208sha256_BYTES_MIN) {
        LogOutput(OT_METHOD)(__func__)(": Too few bytes requested: ")(
            bytes)(" vs "
                   "minimum:"
                   " ")(crypto_pwhash_scryptsalsa208sha256_BYTES_MIN)
            .Flush();

        return false;
    }

    if (bytes > crypto_pwhash_scryptsalsa208sha256_BYTES_MAX) {
        LogOutput(OT_METHOD)(__func__)(": Too many bytes requested: ")(
            bytes)(" vs "
                   "maximum:"
                   " ")(crypto_pwhash_scryptsalsa208sha256_BYTES_MAX)
            .Flush();

        return false;
    }

    auto output = writer(bytes);

    if (false == output.valid(bytes)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to allocated requested ")(
            bytes)(" bytes")
            .Flush();

        return false;
    }

    return 0 == ::crypto_pwhash_scryptsalsa208sha256_ll(
                    reinterpret_cast<const std::uint8_t*>(input.data()),
                    input.size(),
                    reinterpret_cast<const std::uint8_t*>(salt.data()),
                    salt.size(),
                    N,
                    r,
                    p,
                    output.as<std::uint8_t>(),
                    output.size());
}

auto Sodium::HMAC(
    const crypto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    const std::uint8_t* key,
    const size_t keySize,
    std::uint8_t* output) const -> bool
{
    switch (hashType) {
        case (crypto::HashType::Blake2b160):
        case (crypto::HashType::Blake2b256):
        case (crypto::HashType::Blake2b512): {
            return (
                0 == ::crypto_generichash(
                         output,
                         HashingProvider::HashSize(hashType),
                         input,
                         inputSize,
                         key,
                         keySize));
        }
        case (crypto::HashType::Sha256): {
            auto success{false};
            auto state = crypto_auth_hmacsha256_state{};
            success = (0 == crypto_auth_hmacsha256_init(&state, key, keySize));

            if (false == success) {
                LogOutput(OT_METHOD)(__func__)(": Failed to initialize hash")
                    .Flush();

                return false;
            }

            success =
                (0 == crypto_auth_hmacsha256_update(&state, input, inputSize));

            if (false == success) {
                LogOutput(OT_METHOD)(__func__)(": Failed to update hash")
                    .Flush();

                return false;
            }

            return (0 == ::crypto_auth_hmacsha256_final(&state, output));
        }
        case (crypto::HashType::Sha512): {
            auto success{false};
            auto state = crypto_auth_hmacsha512_state{};
            success = (0 == crypto_auth_hmacsha512_init(&state, key, keySize));

            if (false == success) {
                LogOutput(OT_METHOD)(__func__)(": Failed to initialize hash")
                    .Flush();

                return false;
            }

            success =
                (0 == crypto_auth_hmacsha512_update(&state, input, inputSize));

            if (false == success) {
                LogOutput(OT_METHOD)(__func__)(": Failed to update hash")
                    .Flush();

                return false;
            }

            return (0 == ::crypto_auth_hmacsha512_final(&state, output));
        }
        case (crypto::HashType::SipHash24): {
            if (crypto_shorthash_KEYBYTES < keySize) {
                LogOutput(OT_METHOD)(__func__)(": Incorrect key size: ")(
                    keySize)(" vs "
                             "expected"
                             " ")(crypto_shorthash_KEYBYTES)
                    .Flush();

                return false;
            }

            auto buf = std::array<std::uint8_t, crypto_shorthash_KEYBYTES>{};
            std::memcpy(buf.data(), key, keySize);

            return 0 ==
                   ::crypto_shorthash(output, input, inputSize, buf.data());
        }
        default: {
        }
    }

    LogOutput(OT_METHOD)(__func__)(": Unsupported hash function.").Flush();

    return false;
}

auto Sodium::IvSize(const opentxs::crypto::key::symmetric::Algorithm mode) const
    -> std::size_t
{
    switch (mode) {
        case (opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305): {
            return crypto_aead_chacha20poly1305_IETF_NPUBBYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unsupported encryption mode (")(
                value(mode))(").")
                .Flush();
        }
    }
    return 0;
}

auto Sodium::KeySize(
    const opentxs::crypto::key::symmetric::Algorithm mode) const -> std::size_t
{
    switch (mode) {
        case (opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305): {
            return crypto_aead_chacha20poly1305_IETF_KEYBYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unsupported encryption mode (")(
                value(mode))(").")
                .Flush();
        }
    }
    return 0;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
auto Sodium::PubkeyAdd(
    [[maybe_unused]] const ReadView pubkey,
    [[maybe_unused]] const ReadView scalar,
    [[maybe_unused]] const AllocateOutput result) const noexcept -> bool
{
    LogOutput(OT_METHOD)(__func__)(": Not implemented").Flush();

    return false;
}

auto Sodium::RandomKeypair(
    const AllocateOutput privateKey,
    const AllocateOutput publicKey,
    const opentxs::crypto::key::asymmetric::Role,
    const NymParameters&,
    const AllocateOutput) const noexcept -> bool
{
    auto seed = Context().Factory().Secret(0);
    seed->Randomize(crypto_sign_SEEDBYTES);

    return sodium::ExpandSeed(seed->Bytes(), privateKey, publicKey);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

auto Sodium::RandomizeMemory(void* destination, const std::size_t size) const
    -> bool
{
    ::randombytes_buf(destination, size);

    return true;
}

auto Sodium::SaltSize(const crypto::key::symmetric::Source type) const
    -> std::size_t
{
    switch (type) {
        case crypto::key::symmetric::Source::Argon2i:
        case crypto::key::symmetric::Source::Argon2id: {

            return crypto_pwhash_SALTBYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unsupported key type (")(
                value(type))(").")
                .Flush();
        }
    }

    return 0;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
auto Sodium::ScalarAdd(
    const ReadView lhs,
    const ReadView rhs,
    const AllocateOutput result) const noexcept -> bool
{
    if (false == bool(result)) {
        LogOutput(OT_METHOD)(__func__)(": Invalid output allocator").Flush();

        return false;
    }

    if (crypto_core_ed25519_SCALARBYTES != lhs.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid lhs scalar").Flush();

        return false;
    }

    if (crypto_core_ed25519_SCALARBYTES != rhs.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid rhs scalar").Flush();

        return false;
    }

    auto key = result(crypto_core_ed25519_SCALARBYTES);

    if (false == key.valid(crypto_core_ed25519_SCALARBYTES)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to allocate space for result")
            .Flush();

        return false;
    }

    ::crypto_core_ed25519_scalar_add(
        key.as<unsigned char>(),
        reinterpret_cast<const unsigned char*>(lhs.data()),
        reinterpret_cast<const unsigned char*>(rhs.data()));

    return true;
}

auto Sodium::ScalarMultiplyBase(
    const ReadView scalar,
    const AllocateOutput result) const noexcept -> bool
{
    if (false == bool(result)) {
        LogOutput(OT_METHOD)(__func__)(": Invalid output allocator").Flush();

        return false;
    }

    if (crypto_scalarmult_ed25519_SCALARBYTES != scalar.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid scalar").Flush();

        return false;
    }

    auto pub = result(crypto_scalarmult_ed25519_BYTES);

    if (false == pub.valid(crypto_scalarmult_ed25519_BYTES)) {
        LogOutput(OT_METHOD)(__func__)(
            ": Failed to allocate space for public key")
            .Flush();

        return false;
    }

    return 0 == ::crypto_scalarmult_ed25519_base(
                    pub.as<unsigned char>(),
                    reinterpret_cast<const unsigned char*>(scalar.data()));
}

auto Sodium::SharedSecret(
    const ReadView pub,
    const ReadView prv,
    const SecretStyle style,
    Secret& secret) const noexcept -> bool
{
    if (SecretStyle::Default != style) {
        LogOutput(OT_METHOD)(__func__)(": Unsupported secret style").Flush();

        return false;
    }

    if (crypto_sign_PUBLICKEYBYTES != pub.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid public key ").Flush();
        LogOutput(OT_METHOD)(__func__)(": Expected: ")(
            crypto_sign_PUBLICKEYBYTES)
            .Flush();
        LogOutput(OT_METHOD)(__func__)(": Actual:   ")(pub.size()).Flush();

        return false;
    }

    if (crypto_sign_SECRETKEYBYTES != prv.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid private key").Flush();
        LogOutput(OT_METHOD)(__func__)(": Expected: ")(
            crypto_sign_SECRETKEYBYTES)
            .Flush();
        LogOutput(OT_METHOD)(__func__)(": Actual:   ")(prv.size()).Flush();

        return false;
    }

    auto privateEd = Context().Factory().Secret(0);
    auto privateBytes = privateEd->WriteInto(Secret::Mode::Mem)(
        crypto_scalarmult_curve25519_BYTES);
    auto secretBytes =
        secret.WriteInto(Secret::Mode::Mem)(crypto_scalarmult_curve25519_BYTES);
    auto publicEd =
        std::array<unsigned char, crypto_scalarmult_curve25519_BYTES>{};

    OT_ASSERT(privateBytes.valid(crypto_scalarmult_curve25519_BYTES));
    OT_ASSERT(secretBytes.valid(crypto_scalarmult_curve25519_BYTES));

    if (0 != ::crypto_sign_ed25519_pk_to_curve25519(
                 publicEd.data(),
                 reinterpret_cast<const unsigned char*>(pub.data()))) {
        LogOutput(OT_METHOD)(__func__)(
            ": crypto_sign_ed25519_pk_to_curve25519 error")
            .Flush();

        return false;
    }

    if (0 != ::crypto_sign_ed25519_sk_to_curve25519(
                 reinterpret_cast<unsigned char*>(privateBytes.data()),
                 reinterpret_cast<const unsigned char*>(prv.data()))) {
        LogOutput(OT_METHOD)(__func__)(
            ": crypto_sign_ed25519_sk_to_curve25519 error")
            .Flush();

        return false;
    }

    OT_ASSERT(crypto_scalarmult_SCALARBYTES == privateEd->size());
    OT_ASSERT(crypto_scalarmult_BYTES == publicEd.size());
    OT_ASSERT(crypto_scalarmult_BYTES == secret.size());

    return 0 == ::crypto_scalarmult(
                    static_cast<unsigned char*>(secretBytes.data()),
                    static_cast<const unsigned char*>(privateBytes.data()),
                    publicEd.data());
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

auto Sodium::sha1(
    const std::uint8_t* input,
    const std::size_t size,
    std::uint8_t* output) const -> bool
{
    if (std::numeric_limits<std::uint32_t>::max() < size) { return false; }

    auto hex = std::array<char, SHA1_HEX_SIZE>{};
    ::sha1()
        .add(input, static_cast<std::uint32_t>(size))
        .finalize()
        .print_hex(hex.data());
    const auto hash = Data::Factory(hex.data(), Data::Mode::Hex);
    std::memcpy(output, hash->data(), hash->size());

    return true;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
auto Sodium::Sign(
    const ReadView plaintext,
    const ReadView priv,
    const crypto::HashType hash,
    const AllocateOutput signature) const -> bool
{
    if (crypto::HashType::Blake2b256 != hash) {
        LogVerbose(OT_METHOD)(__func__)(": Unsupported hash function: ")(
            value(hash))
            .Flush();

        return false;
    }

    if (nullptr == priv.data() || 0 == priv.size()) {
        LogOutput(OT_METHOD)(__func__)(": Missing private key").Flush();

        return false;
    }

    if (crypto_sign_SECRETKEYBYTES != priv.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid private key").Flush();
        LogOutput(OT_METHOD)(__func__)(": Expected: ")(
            crypto_sign_SECRETKEYBYTES)
            .Flush();
        LogOutput(OT_METHOD)(__func__)(": Actual:   ")(priv.size()).Flush();

        return false;
    }

    if (priv == blank_private()) {
        LogOutput(OT_METHOD)(__func__)(": Blank private key").Flush();

        return false;
    }

    if (false == bool(signature)) {
        LogOutput(OT_METHOD)(__func__)(": Invalid output allocator").Flush();

        return false;
    }

    auto output = signature(crypto_sign_BYTES);

    if (false == output.valid(crypto_sign_BYTES)) {
        LogOutput(OT_METHOD)(__func__)(
            ": Failed to allocate space for signature")
            .Flush();

        return false;
    }

    const auto success =
        0 == ::crypto_sign_detached(
                 output.as<unsigned char>(),
                 nullptr,
                 reinterpret_cast<const unsigned char*>(plaintext.data()),
                 plaintext.size(),
                 reinterpret_cast<const unsigned char*>(priv.data()));

    if (false == success) {
        LogOutput(OT_METHOD)(__func__)(": Failed to sign plaintext.").Flush();
    }

    return success;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

auto Sodium::TagSize(
    const opentxs::crypto::key::symmetric::Algorithm mode) const -> std::size_t
{
    switch (mode) {
        case (opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305): {
            return crypto_aead_chacha20poly1305_IETF_ABYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unsupported encryption mode (")(
                value(mode))(").")
                .Flush();
        }
    }
    return 0;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
auto Sodium::Verify(
    const ReadView plaintext,
    const ReadView pub,
    const ReadView signature,
    const crypto::HashType type) const -> bool
{
    if (crypto::HashType::Blake2b256 != type) {
        LogVerbose(OT_METHOD)(__func__)(": Unsupported hash function: ")(
            value(type))
            .Flush();

        return false;
    }

    if (crypto_sign_BYTES != signature.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid signature").Flush();

        return false;
    }

    if (nullptr == pub.data() || 0 == pub.size()) {
        LogOutput(OT_METHOD)(__func__)(": Missing public key").Flush();

        return false;
    }

    if (crypto_sign_PUBLICKEYBYTES != pub.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid public key").Flush();
        LogOutput(OT_METHOD)(__func__)(": Expected: ")(
            crypto_sign_PUBLICKEYBYTES)
            .Flush();
        LogOutput(OT_METHOD)(__func__)(": Actual:   ")(pub.size()).Flush();

        return false;
    }

    const auto success =
        0 == ::crypto_sign_verify_detached(
                 reinterpret_cast<const unsigned char*>(signature.data()),
                 reinterpret_cast<const unsigned char*>(plaintext.data()),
                 plaintext.size(),
                 reinterpret_cast<const unsigned char*>(pub.data()));

    if (false == success) {
        LogVerbose(OT_METHOD)(__func__)(": Failed to verify signature").Flush();
    }

    return success;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
}  // namespace opentxs::crypto::implementation

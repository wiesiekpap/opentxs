// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_SYMMETRICPROVIDER_HPP
#define OPENTXS_CRYPTO_LIBRARY_SYMMETRICPROVIDER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace proto
{
class Ciphertext;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace crypto
{
class OPENTXS_EXPORT SymmetricProvider
{
public:
    OPENTXS_NO_EXPORT virtual auto Decrypt(
        const proto::Ciphertext& ciphertext,
        const std::uint8_t* key,
        const std::size_t keySize,
        std::uint8_t* plaintext) const -> bool = 0;
    virtual auto DefaultMode() const
        -> opentxs::crypto::key::symmetric::Algorithm = 0;
    virtual auto Derive(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* salt,
        const std::size_t saltSize,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::uint64_t parallel,
        const crypto::key::symmetric::Source type,
        std::uint8_t* output,
        std::size_t outputSize) const -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Encrypt(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* key,
        const std::size_t keySize,
        proto::Ciphertext& ciphertext) const -> bool = 0;
    virtual auto IvSize(const opentxs::crypto::key::symmetric::Algorithm mode)
        const -> std::size_t = 0;
    virtual auto KeySize(const opentxs::crypto::key::symmetric::Algorithm mode)
        const -> std::size_t = 0;
    virtual auto SaltSize(const crypto::key::symmetric::Source type) const
        -> std::size_t = 0;
    virtual auto TagSize(const opentxs::crypto::key::symmetric::Algorithm mode)
        const -> std::size_t = 0;

    OPENTXS_NO_EXPORT virtual ~SymmetricProvider() = default;

protected:
    SymmetricProvider() = default;

private:
    SymmetricProvider(const SymmetricProvider&) = delete;
    SymmetricProvider(SymmetricProvider&&) = delete;
    auto operator=(const SymmetricProvider&) -> SymmetricProvider& = delete;
    auto operator=(SymmetricProvider&&) -> SymmetricProvider& = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif

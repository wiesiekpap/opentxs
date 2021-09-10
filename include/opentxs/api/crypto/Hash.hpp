// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_HASH_HPP
#define OPENTXS_API_CRYPTO_HASH_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network

class Data;
class Secret;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace crypto
{
class OPENTXS_EXPORT Hash
{
public:
    virtual auto Digest(
        const opentxs::crypto::HashType hashType,
        const ReadView data,
        const AllocateOutput destination) const noexcept -> bool = 0;
    virtual auto Digest(
        const opentxs::crypto::HashType hashType,
        const opentxs::network::zeromq::Frame& data,
        const AllocateOutput destination) const noexcept -> bool = 0;
    virtual auto Digest(
        const std::uint32_t type,
        const ReadView data,
        const AllocateOutput encodedDestination) const noexcept -> bool = 0;
    virtual auto HMAC(
        const opentxs::crypto::HashType hashType,
        const ReadView key,
        const ReadView& data,
        const AllocateOutput digest) const noexcept -> bool = 0;
    virtual void MurmurHash3_32(
        const std::uint32_t& key,
        const Data& data,
        std::uint32_t& output) const noexcept = 0;
    virtual auto PKCS5_PBKDF2_HMAC(
        const Data& input,
        const Data& salt,
        const std::size_t iterations,
        const opentxs::crypto::HashType hashType,
        const std::size_t bytes,
        Data& output) const noexcept -> bool = 0;
    virtual auto PKCS5_PBKDF2_HMAC(
        const Secret& input,
        const Data& salt,
        const std::size_t iterations,
        const opentxs::crypto::HashType hashType,
        const std::size_t bytes,
        Data& output) const noexcept -> bool = 0;
    virtual auto PKCS5_PBKDF2_HMAC(
        const std::string& input,
        const Data& salt,
        const std::size_t iterations,
        const opentxs::crypto::HashType hashType,
        const std::size_t bytes,
        Data& output) const noexcept -> bool = 0;
    virtual auto Scrypt(
        const ReadView input,
        const ReadView salt,
        const std::uint64_t N,
        const std::uint32_t r,
        const std::uint32_t p,
        const std::size_t bytes,
        AllocateOutput writer) const noexcept -> bool = 0;

    OPENTXS_NO_EXPORT virtual ~Hash() = default;

protected:
    Hash() noexcept = default;

private:
    Hash(const Hash&) = delete;
    Hash(Hash&&) = delete;
    auto operator=(const Hash&) -> Hash& = delete;
    auto operator=(Hash&&) -> Hash& = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif

//
// Created by PetterFi on 28.07.22.
//

#pragma once

#include "internal/crypto/library/Keccak.hpp"

#include <cstddef>
#include <cstdint>

namespace opentxs::crypto::implementation
{
class Keccak final : virtual public crypto::Keccak
{
public:
    static constexpr int HASH_BYTES_NUMBER = 32;

    static void getHash(
        const std::uint8_t msg[],
        std::size_t len,
        std::uint8_t hashResult[HASH_BYTES_NUMBER]);

    auto Digest(
        const crypto::HashType hashType,
        const ReadView data,
        const AllocateOutput output) const noexcept -> bool final;
    auto HMAC(
        const crypto::HashType hashType,
        const ReadView key,
        const ReadView data,
        const AllocateOutput output) const noexcept -> bool final;

    Keccak() noexcept;

    ~Keccak() final = default;

private:
    static constexpr int HASH_CHAR_NUMBER = HASH_BYTES_NUMBER * 2;
    static constexpr int BLOCK_SIZE = 200 - HASH_BYTES_NUMBER * 2;
    static constexpr int NUM_ROUNDS = 24;
    static const unsigned char ROTATION[5][5];

    static void absorb(std::uint64_t state[5][5]);
    // Requires 0 <= i <= 63
    static std::uint64_t rotl64(std::uint64_t x, int i);

    Keccak(const Keccak&) = delete;
    Keccak(Keccak&&) = delete;
    auto operator=(const Keccak&) -> Keccak& = delete;
    auto operator=(Keccak&&) -> Keccak& = delete;
};
}  // namespace opentxs::crypto::implementation

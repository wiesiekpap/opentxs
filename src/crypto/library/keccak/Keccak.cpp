//
// Created by PetterFi on 28.07.22.
//
// based on
// https://github.com/nayuki/Bitcoin-Cryptography-Library

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "crypto/library/keccak/Keccak.hpp"  // IWYU pragma: associated

#include <cassert>

#include "internal/crypto/library/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"

using std::size_t;
using std::uint64_t;
using std::uint8_t;

namespace opentxs::factory
{
auto Keccak() noexcept -> std::unique_ptr<crypto::Keccak>
{
    using ReturnType = crypto::implementation::Keccak;

    return std::make_unique<ReturnType>();
}
}  // namespace opentxs::factory

namespace opentxs::crypto::implementation
{
Keccak::Keccak() noexcept {}

auto Keccak::Digest(
    const crypto::HashType hashType,
    const ReadView data,
    const AllocateOutput output) const noexcept -> bool
{
    try {
        if (false == output.operator bool()) {
            throw std::runtime_error{"invalid output"};
        }

        const auto size = HashSize(hashType);
        auto buf = output(size);

        if (false == buf.valid(size)) {
            throw std::runtime_error{"failed to allocate space for output"};
        }

        auto* uint_data_array = new uint8_t[HASH_CHAR_NUMBER];
        memcpy(
            uint_data_array,
            reinterpret_cast<const char*>(data.data()),
            HASH_CHAR_NUMBER);

        getHash(uint_data_array, data.size(), buf.as<uint8_t>());
        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what())("Cannot keccak hash").Flush();

        return false;
    }
}

auto Keccak::HMAC(
    const crypto::HashType hashType,
    const ReadView key,
    const ReadView data,
    const AllocateOutput output) const noexcept -> bool
{
    return false;
}

void Keccak::getHash(
    const uint8_t msg[],
    size_t len,
    uint8_t hashResult[HASH_BYTES_NUMBER])
{
    assert((msg != nullptr || len == 0) && hashResult != nullptr);
    uint64_t state[5][5] = {};

    // XOR each message byte into the state, and absorb full blocks
    int blockOff = 0;
    for (size_t i = 0; i < len; i++) {
        int j = blockOff >> 3;
        state[j % 5][j / 5] ^= static_cast<uint64_t>(msg[i])
                               << ((blockOff & 7) << 3);
        blockOff++;
        if (blockOff == BLOCK_SIZE) {
            absorb(state);
            blockOff = 0;
        }
    }

    // Final block and padding
    {
        int i = blockOff >> 3;
        state[i % 5][i / 5] ^= UINT64_C(0x01) << ((blockOff & 7) << 3);
        blockOff = BLOCK_SIZE - 1;
        int j = blockOff >> 3;
        state[j % 5][j / 5] ^= UINT64_C(0x80) << ((blockOff & 7) << 3);
        absorb(state);
    }

    // Uint64 array to bytes in little endian
    for (int i = 0; i < HASH_BYTES_NUMBER; i++) {
        int j = i >> 3;
        hashResult[i] =
            static_cast<uint8_t>(state[j % 5][j / 5] >> ((i & 7) << 3));
    }
}

void Keccak::absorb(uint64_t state[5][5])
{
    uint64_t(*a)[5] = state;
    uint8_t r = 1;  // LFSR
    for (int i = 0; i < NUM_ROUNDS; i++) {
        // Theta step
        uint64_t c[5] = {};
        for (int x = 0; x < 5; x++) {
            for (int y = 0; y < 5; y++) c[x] ^= a[x][y];
        }
        for (int x = 0; x < 5; x++) {
            uint64_t d = c[(x + 4) % 5] ^ rotl64(c[(x + 1) % 5], 1);
            for (int y = 0; y < 5; y++) a[x][y] ^= d;
        }

        // Rho and pi steps
        uint64_t b[5][5];
        for (int x = 0; x < 5; x++) {
            for (int y = 0; y < 5; y++)
                b[y][(x * 2 + y * 3) % 5] = rotl64(a[x][y], ROTATION[x][y]);
        }

        // Chi step
        for (int x = 0; x < 5; x++) {
            for (int y = 0; y < 5; y++)
                a[x][y] = b[x][y] ^ (~b[(x + 1) % 5][y] & b[(x + 2) % 5][y]);
        }

        // Iota step
        for (int j = 0; j < 7; j++) {
            a[0][0] ^= static_cast<uint64_t>(r & 1) << ((1 << j) - 1);
            r = static_cast<uint8_t>((r << 1) ^ ((r >> 7) * 0x171));
        }
    }
}

uint64_t Keccak::rotl64(uint64_t x, int i)
{
    return ((0U + x) << i) | (x >> ((64 - i) & 63));
}

// Static initializers
const unsigned char Keccak::ROTATION[5][5] = {
    {0, 36, 3, 41, 18},
    {1, 44, 10, 45, 2},
    {62, 6, 43, 15, 61},
    {28, 55, 25, 21, 56},
    {27, 20, 39, 8, 14},
};
}  // namespace opentxs::crypto::implementation

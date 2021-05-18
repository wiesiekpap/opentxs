// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <tuple>
#include <vector>

#include "blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api
}  // namespace opentxs

namespace be = boost::endian;

namespace opentxs::blockchain::bitcoin
{
/// output: the decoded value
///
/// input: gets incremented to the byte past the CompactSize
///
/// expectedSize: gets incremented by csBytes
///
/// csExtraBytes: additional bytes consumed by the CompactSize beyond the
/// minimum of 1 byte. For small encoded sizes, this will be 0.
///
/// A CompactSize is at least one byte, so before you attempt to execute this
/// this function position input to the first byte and set expectedSize
/// appropriately to make sure the input contains at least one byte.
/// If the CompactSize is larger than 1 byte, then expectedSize will be updated.
///
/// In all cases input will be advanced as needed.
///
/// Normally you don't need to worry about how many bytes the CompactSize
/// actually occupied because you're just going to move on to reading the next
/// element, but just in case you do there's an overload which outputs
/// csExtraBytes
using Byte = const std::byte;
using ByteIterator = Byte*;

OPENTXS_EXPORT auto DecodeCompactSizeFromPayload(
    ByteIterator& input,
    std::size_t& expectedSize,
    const std::size_t size,
    std::size_t& output) noexcept -> bool;
auto DecodeCompactSizeFromPayload(
    ByteIterator& input,
    std::size_t& expectedSize,
    const std::size_t size,
    CompactSize& output) noexcept -> bool;
auto DecodeCompactSizeFromPayload(
    ByteIterator& input,
    std::size_t& expectedSize,
    const std::size_t size,
    std::size_t& output,
    std::size_t& csExtraBytes) noexcept -> bool;

/// input: gets incremented to the byte past the segwit flag byte if transaction
/// is segwit
///
/// expectedSize: gets incremented by 2 if transaction is segwit
auto HasSegwit(
    ByteIterator& input,
    std::size_t& expectedSize,
    const std::size_t size) noexcept(false) -> std::optional<std::byte>;

struct EncodedOutpoint {
    std::array<std::byte, 32> txid_{};
    be::little_uint32_buf_t index_{};
};

struct EncodedInput {
    EncodedOutpoint outpoint_{};
    CompactSize cs_{};
    Space script_{};
    be::little_uint32_buf_t sequence_{};

    auto size() const noexcept -> std::size_t;
};

struct EncodedOutput {
    be::little_uint64_buf_t value_{};
    CompactSize cs_{};
    Space script_{};

    auto size() const noexcept -> std::size_t;
};

struct EncodedWitnessItem {
    CompactSize cs_{};
    Space item_{};

    auto size() const noexcept -> std::size_t;
};

struct EncodedInputWitness {
    CompactSize cs_{};
    std::vector<EncodedWitnessItem> items_{};

    auto size() const noexcept -> std::size_t;
};

struct OPENTXS_EXPORT EncodedTransaction {
    be::little_int32_buf_t version_{};
    std::optional<std::byte> segwit_flag_{};
    CompactSize input_count_{};
    std::vector<EncodedInput> inputs_{};
    CompactSize output_count_{};
    std::vector<EncodedOutput> outputs_{};
    std::vector<EncodedInputWitness> witnesses_{};
    be::little_uint32_buf_t lock_time_{};
    Space wtxid_{};
    Space txid_{};

    static auto DefaultVersion(const blockchain::Type chain) noexcept
        -> std::uint32_t;

    auto CalculateIDs(
        const api::Core& api,
        const blockchain::Type chain) noexcept -> bool;
    auto CalculateIDs(
        const api::Core& api,
        const blockchain::Type chain,
        ReadView bytes) noexcept -> bool;
    static auto Deserialize(
        const api::Core& api,
        const blockchain::Type chain,
        const ReadView bytes) noexcept(false) -> EncodedTransaction;

    auto wtxid_preimage() const noexcept -> Space;
    auto txid_preimage() const noexcept -> Space;
    auto txid_size() const noexcept -> std::size_t;
    auto size() const noexcept -> std::size_t;
};

enum class SigOption : std::uint8_t {
    All,
    None,
    Single,
};

struct SigHash {
    std::byte flags_{0x01};
    std::array<std::byte, 3> forkid_{};

    auto AnyoneCanPay() const noexcept -> bool;
    auto begin() const noexcept -> const std::byte*;
    auto end() const noexcept -> const std::byte*;
    auto ForkID() const noexcept -> ReadView;
    auto Type() const noexcept -> SigOption;

    SigHash(
        const blockchain::Type chain,
        const SigOption flag = SigOption::All,
        const bool anyoneCanPay = false) noexcept;
};

struct Bip143Hashes {
    using Hash = std::array<std::byte, 32>;

    Hash outpoints_{};
    Hash sequences_{};
    Hash outputs_{};

    auto Outpoints(const SigHash type) const noexcept -> const Hash&;
    auto Outputs(const SigHash type, const Hash* single) const noexcept
        -> const Hash&;
    auto Sequences(const SigHash type) const noexcept -> const Hash&;

private:
    static auto blank() noexcept -> const Hash&;
};
}  // namespace opentxs::blockchain::bitcoin

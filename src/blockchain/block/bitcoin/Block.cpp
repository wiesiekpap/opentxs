// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/block/bitcoin/Block.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "blockchain/block/Block.hpp"
#include "blockchain/block/bitcoin/BlockParser.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/iterator/Bidirectional.hpp"
#include "util/Container.hpp"

#define OT_METHOD "opentxs::blockchain::block::bitcoin::implementation::Block::"

namespace be = boost::endian;

namespace opentxs::factory
{
auto BitcoinBlock(
    const api::Core& api,
    const opentxs::blockchain::block::Header& previous,
    const Transaction_p pGen,
    const std::uint32_t nBits,
    const std::vector<Transaction_p>& extra,
    const std::int32_t version,
    const AbortFunction abort) noexcept
    -> std::shared_ptr<const opentxs::blockchain::block::bitcoin::Block>
{
    try {
        using Block = blockchain::block::bitcoin::implementation::Block;

        if (false == bool(pGen)) {
            throw std::runtime_error{"Invalid generation transaction"};
        }

        const auto& gen = *pGen;
        using Tx = blockchain::block::bitcoin::Transaction;

        {
            auto& mGen = const_cast<Tx&>(gen);
            mGen.SetPosition(0);
        }

        auto index = Block::TxidIndex{};
        auto map = Block::TransactionMap{};
        auto position = std::size_t{0};

        {
            const auto& id = gen.ID();
            index.emplace_back(id.begin(), id.end());
            const auto& item = index.back();
            map.emplace(reader(item), pGen);
        }

        for (const auto& tx : extra) {
            if (false == bool(tx)) {
                throw std::runtime_error{"Invalid transaction"};
            }

            {
                auto& mTx = const_cast<Tx&>(*tx);
                mTx.SetPosition(++position);
            }

            const auto& id = tx->ID();
            index.emplace_back(id.begin(), id.end());
            const auto& item = index.back();
            map.emplace(reader(item), tx);
        }

        const auto chain = previous.Type();
        auto header = BitcoinBlockHeader(
            api,
            previous,
            nBits,
            version,
            Block::calculate_merkle_value(api, chain, index),
            abort);

        if (false == bool(header)) {
            throw std::runtime_error{"Failed to create block header"};
        }

        switch (chain) {
            case blockchain::Type::Bitcoin:
            case blockchain::Type::Bitcoin_testnet3:
            case blockchain::Type::BitcoinCash:
            case blockchain::Type::BitcoinCash_testnet3:
            case blockchain::Type::Litecoin:
            case blockchain::Type::Litecoin_testnet4:
            case blockchain::Type::UnitTest: {

                return std::make_shared<Block>(
                    api,
                    chain,
                    std::move(header),
                    std::move(index),
                    std::move(map));
            }
            case blockchain::Type::PKT:
            case blockchain::Type::PKT_testnet: {
                // TODO
                return {};
            }
            case blockchain::Type::Unknown:
            case blockchain::Type::Ethereum_frontier:
            case blockchain::Type::Ethereum_ropsten:
            default: {
                LogOutput(OT_METHOD)(__func__)(": Unsupported type (")(
                    static_cast<std::uint32_t>(chain))(")")
                    .Flush();

                return {};
            }
        }
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
auto BitcoinBlock(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const ReadView in) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Block>
{
    try {
        switch (chain) {
            case blockchain::Type::Bitcoin:
            case blockchain::Type::Bitcoin_testnet3:
            case blockchain::Type::BitcoinCash:
            case blockchain::Type::BitcoinCash_testnet3:
            case blockchain::Type::Litecoin:
            case blockchain::Type::Litecoin_testnet4:
            case blockchain::Type::UnitTest: {
                return parse_normal_block(api, blockchain, chain, in);
            }
            case blockchain::Type::PKT:
            case blockchain::Type::PKT_testnet: {
                return parse_pkt_block(api, blockchain, chain, in);
            }
            case blockchain::Type::Unknown:
            case blockchain::Type::Ethereum_frontier:
            case blockchain::Type::Ethereum_ropsten:
            default: {
                LogOutput(OT_METHOD)(__func__)(": Unsupported type (")(
                    static_cast<std::uint32_t>(chain))(")")
                    .Flush();

                return {};
            }
        }
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto parse_normal_block(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const ReadView in) noexcept(false)
    -> std::shared_ptr<blockchain::block::bitcoin::Block>
{
    OT_ASSERT(
        (blockchain::Type::PKT != chain) &&
        (blockchain::Type::PKT_testnet != chain));

    auto it = ByteIterator{};
    auto expectedSize = std::size_t{};
    auto pHeader = parse_header(api, chain, in, it, expectedSize);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;
    auto sizeData = ReturnType::CalculatedSize{
        in.size(), network::blockchain::bitcoin::CompactSize{}};
    auto [index, transactions] = parse_transactions(
        api, blockchain, chain, in, header, sizeData, it, expectedSize);

    return std::make_shared<ReturnType>(
        api,
        chain,
        std::move(pHeader),
        std::move(index),
        std::move(transactions),
        std::move(sizeData));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block
{
Block::ParsedPatterns::ParsedPatterns(const Block::Patterns& in) noexcept
    : data_()
    , map_()
{
    data_.reserve(in.size());

    for (auto i{in.cbegin()}; i != in.cend(); std::advance(i, 1)) {
        const auto& [elementID, data] = *i;
        map_.emplace(reader(data), i);
        data_.emplace_back(data);
    }

    std::sort(data_.begin(), data_.end());
}

auto SetIntersection(
    const api::Core& api,
    const ReadView txid,
    const Block::ParsedPatterns& parsed,
    const std::vector<Space>& compare) noexcept -> Block::Matches
{
    auto matches = std::vector<Space>{};
    auto output = Block::Matches{};
    std::set_intersection(
        std::begin(parsed.data_),
        std::end(parsed.data_),
        std::begin(compare),
        std::end(compare),
        std::back_inserter(matches));
    output.second.reserve(matches.size());
    std::transform(
        std::begin(matches),
        std::end(matches),
        std::back_inserter(output.second),
        [&](const auto& match) -> Block::Match {
            return {
                api.Factory().Data(txid), parsed.map_.at(reader(match))->first};
        });

    return output;
}
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block::bitcoin::internal
{
auto Opcode(const OP opcode) noexcept(false) -> ScriptElement
{
    return {opcode, {}, {}, {}};
}

auto PushData(const ReadView in) noexcept(false) -> ScriptElement
{
    const auto size = in.size();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
    // std::size_t might be 32 bit
    if (size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("Too many bytes");
    }
#pragma GCC diagnostic pop

    if ((nullptr == in.data()) || (0 == size)) {
        return {OP::PUSHDATA1, {}, Space{std::byte{0x0}}, Space{}};
    }

    auto output = ScriptElement{};
    auto& [opcode, invalid, bytes, data] = output;

    if (75 >= size) {
        opcode = static_cast<OP>(static_cast<std::uint8_t>(size));
    } else if (std::numeric_limits<std::uint8_t>::max() >= size) {
        opcode = OP::PUSHDATA1;
        bytes = Space{std::byte{static_cast<std::uint8_t>(size)}};
    } else if (std::numeric_limits<std::uint16_t>::max() >= size) {
        opcode = OP::PUSHDATA2;
        const auto buf =
            be::little_uint16_buf_t{static_cast<std::uint16_t>(size)};
        bytes = space(sizeof(buf));
        std::memcpy(bytes.value().data(), &buf, sizeof(buf));
    } else {
        opcode = OP::PUSHDATA4;
        const auto buf =
            be::little_uint32_buf_t{static_cast<std::uint32_t>(size)};
        bytes = space(sizeof(buf));
        std::memcpy(bytes.value().data(), &buf, sizeof(buf));
    }

    data = space(size);
    std::memcpy(data.value().data(), in.data(), in.size());

    return output;
}
}  // namespace opentxs::blockchain::block::bitcoin::internal

namespace opentxs::blockchain::block::bitcoin::implementation
{
const std::size_t Block::header_bytes_{80};
const Block::value_type Block::null_tx_{};

Block::Block(
    const api::Core& api,
    const blockchain::Type chain,
    std::unique_ptr<const internal::Header> header,
    TxidIndex&& index,
    TransactionMap&& transactions,
    std::optional<CalculatedSize>&& size) noexcept(false)
    : block::implementation::Block(api, *header)
    , header_p_(std::move(header))
    , header_(*header_p_)
    , index_(std::move(index))
    , transactions_(std::move(transactions))
    , size_(std::move(size))
{
    if (index_.size() != transactions_.size()) {
        throw std::runtime_error("Invalid transaction index");
    }

    if (false == bool(header_p_)) {
        throw std::runtime_error("Invalid header");
    }

    for (const auto& [txid, tx] : transactions_) {
        if (false == bool(tx)) {
            throw std::runtime_error("Invalid transaction");
        }
    }
}

auto Block::at(const std::size_t index) const noexcept -> const value_type&
{
    try {
        if (index_.size() <= index) {
            throw std::out_of_range("invalid index " + std::to_string(index));
        }

        return at(reader(index_.at(index)));
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return null_tx_;
    }
}

auto Block::at(const ReadView txid) const noexcept -> const value_type&
{
    try {

        return transactions_.at(txid);
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": transaction ")(
            api_.Factory().Data(txid)->asHex())(" not found in block ")(
            header_.Hash().asHex())
            .Flush();

        return null_tx_;
    }
}

template <typename HashType>
auto Block::calculate_merkle_hash(
    const api::Core& api,
    const Type chain,
    const HashType& lhs,
    const HashType& rhs,
    AllocateOutput out) -> bool
{
    auto preimage = std::array<std::byte, 64>{};
    constexpr auto chunk = preimage.size() / 2u;

    if (chunk != lhs.size()) {
        throw std::runtime_error("Invalid lhs hash size");
    }
    if (chunk != rhs.size()) {
        throw std::runtime_error("Invalid rhs hash size");
    }

    auto it = preimage.data();
    std::memcpy(it, lhs.data(), chunk);
    std::advance(it, chunk);
    std::memcpy(it, rhs.data(), chunk);

    return MerkleHash(
        api,
        chain,
        {reinterpret_cast<const char*>(preimage.data()), preimage.size()},
        out);
}

template <typename InputContainer, typename OutputContainer>
auto Block::calculate_merkle_row(
    const api::Core& api,
    const Type chain,
    const InputContainer& in,
    OutputContainer& out) -> bool
{
    out.clear();
    const auto count{in.size()};

    for (auto i = std::size_t{0}; i < count; i += 2u) {
        const auto offset = std::size_t{(1u == (count - i)) ? 0u : 1u};
        auto& next = out.emplace_back();
        const auto hashed = calculate_merkle_hash(
            api,
            chain,
            in.at(i),
            in.at(i + offset),
            preallocated(next.size(), next.data()));

        if (false == hashed) { return false; }
    }

    return true;
}

auto Block::calculate_merkle_value(
    const api::Core& api,
    const Type chain,
    const TxidIndex& txids) -> block::pHash
{
    using Hash = std::array<std::byte, 32>;

    if (0 == txids.size()) {
        constexpr auto blank = Hash{};

        return api.Factory().Data(ReadView{
            reinterpret_cast<const char*>(blank.data()), blank.size()});
    }

    if (1 == txids.size()) { return api.Factory().Data(txids.at(0)); }

    auto a = std::vector<Hash>{};
    auto b = std::vector<Hash>{};
    a.reserve(txids.size());
    b.reserve(txids.size());
    auto counter{0};
    calculate_merkle_row(api, chain, txids, a);

    if (1u == a.size()) { return api.Factory().Data(reader(a.at(0))); }

    while (true) {
        const auto& src = (1 == (++counter % 2)) ? a : b;
        auto& dst = (0 == (counter % 2)) ? a : b;
        calculate_merkle_row(api, chain, src, dst);

        if (1u == dst.size()) { return api.Factory().Data(reader(dst.at(0))); }
    }
}

auto Block::calculate_size() const noexcept -> CalculatedSize
{
    auto output = CalculatedSize{
        0, network::blockchain::bitcoin::CompactSize(transactions_.size())};
    auto& [bytes, cs] = output;
    auto cb = [](const auto& previous, const auto& in) -> std::size_t {
        return previous + in.second->CalculateSize();
    };
    bytes = std::accumulate(
        std::begin(transactions_),
        std::end(transactions_),
        header_bytes_ + cs.Size() + extra_bytes(),
        cb);

    return output;
}

auto Block::ExtractElements(const FilterType style) const noexcept
    -> std::vector<Space>
{
    auto output = std::vector<Space>{};
    LogTrace(OT_METHOD)(__func__)(": processing ")(transactions_.size())(
        " transactions")
        .Flush();

    for (const auto& [txid, tx] : transactions_) {
        auto temp = tx->ExtractElements(style);
        output.insert(
            output.end(),
            std::make_move_iterator(temp.begin()),
            std::make_move_iterator(temp.end()));
    }

    LogTrace(OT_METHOD)(__func__)(": extracted ")(output.size())(" elements")
        .Flush();
    std::sort(output.begin(), output.end());

    return output;
}

auto Block::FindMatches(
    const FilterType style,
    const Patterns& outpoints,
    const Patterns& patterns) const noexcept -> Matches
{
    if (0 == (outpoints.size() + patterns.size())) { return {}; }

    LogTrace(OT_METHOD)(__func__)(": Verifying ")(
        patterns.size() + outpoints.size())(" potential matches in ")(
        transactions_.size())(" transactions")
        .Flush();
    auto output = Matches{};
    auto& [inputs, outputs] = output;
    const auto parsed = ParsedPatterns{patterns};

    for (const auto& [txid, tx] : transactions_) {
        auto temp = tx->FindMatches(style, outpoints, parsed);
        inputs.insert(
            inputs.end(),
            std::make_move_iterator(temp.first.begin()),
            std::make_move_iterator(temp.first.end()));
        outputs.insert(
            outputs.end(),
            std::make_move_iterator(temp.second.begin()),
            std::make_move_iterator(temp.second.end()));
    }

    dedup(inputs);
    dedup(outputs);

    return output;
}

auto Block::get_or_calculate_size() const noexcept -> CalculatedSize
{
    if (false == size_.has_value()) { size_ = calculate_size(); }

    OT_ASSERT(size_.has_value());

    return size_.value();
}

auto Block::Print() const noexcept -> std::string
{
    auto out = std::stringstream{};
    out << "header" << '\n' << header_.Print();
    auto count{0};
    const auto total = size();

    for (const auto& tx : *this) {
        out << "transaction " << std::to_string(++count);
        out << " of " << std::to_string(total) << '\n';
        out << tx->Print();
    }

    return out.str();
}

auto Block::Serialize(AllocateOutput bytes) const noexcept -> bool
{
    if (false == bool(bytes)) {
        LogOutput(OT_METHOD)(__func__)(": Invalid output allocator").Flush();

        return false;
    }

    const auto [size, txCount] = get_or_calculate_size();
    const auto out = bytes(size);

    if (false == out.valid(size)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to allocate output").Flush();

        return false;
    }

    LogInsane(OT_METHOD)(__func__)(": Serializing ")(txCount.Value())(
        " transactions into ")(size)(" bytes.")
        .Flush();
    auto remaining = std::size_t{size};
    auto it = static_cast<std::byte*>(out.data());

    if (false == header_.Serialize(preallocated(remaining, it))) {
        LogOutput(OT_METHOD)(__func__)(": Failed to serialize header").Flush();

        return false;
    }

    remaining -= header_bytes_;
    std::advance(it, header_bytes_);

    if (false == serialize_post_header(it, remaining)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to extra data (post header)")
            .Flush();

        return false;
    }

    if (false == txCount.Encode(preallocated(remaining, it))) {
        LogOutput(OT_METHOD)(__func__)(
            ": Failed to serialize transaction count")
            .Flush();

        return false;
    }

    remaining -= txCount.Size();
    std::advance(it, txCount.Size());

    for (const auto& txid : index_) {
        try {
            const auto& pTX = transactions_.at(reader(txid));

            OT_ASSERT(pTX);

            const auto& tx = *pTX;
            const auto encoded = tx.Serialize(preallocated(remaining, it));

            if (false == encoded.has_value()) {
                LogOutput(OT_METHOD)(__func__)(
                    ": failed to serialize transaction ")(tx.ID().asHex())
                    .Flush();

                return false;
            }

            remaining -= encoded.value();
            std::advance(it, encoded.value());
        } catch (...) {
            LogOutput(OT_METHOD)(__func__)(": missing transaction").Flush();

            return false;
        }
    }

    if (0 != remaining) {
        LogOutput(OT_METHOD)(__func__)(": Extra bytes: ")(remaining).Flush();

        return false;
    }

    return true;
}

auto Block::serialize_post_header(
    [[maybe_unused]] ByteIterator& it,
    [[maybe_unused]] std::size_t& remaining) const noexcept -> bool
{
    return true;
}

Block::~Block() = default;
}  // namespace opentxs::blockchain::block::bitcoin::implementation

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <robin_hood.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Opcodes.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace be = boost::endian;

namespace opentxs::factory
{
auto BitcoinScript(
    const blockchain::Type chain,
    const ReadView bytes,
    const blockchain::block::bitcoin::Script::Position role,
    const bool allowInvalidOpcodes,
    const bool mute) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Script>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Script;
    auto elements = blockchain::block::bitcoin::ScriptElements{};

    if ((nullptr == bytes.data()) || (0 == bytes.size()) ||
        (ReturnType::Position::Coinbase == role)) {
        return std::make_unique<ReturnType>(
            chain, role, std::move(elements), 0);
    }

    elements.reserve(bytes.size());
    auto it = reinterpret_cast<const std::byte*>(bytes.data());
    auto read = std::size_t{0};
    const auto target = bytes.size();
    const auto& logger = mute ? LogTrace() : LogVerbose();

    try {
        while (read < target) {
            auto& element = elements.emplace_back();
            auto& [opcode, invalid, size, data] = element;

            try {
                opcode = ReturnType::decode(*it);
            } catch (...) {
                if (allowInvalidOpcodes) {
                    opcode = blockchain::block::bitcoin::OP::INVALIDOPCODE;
                    invalid = *it;
                } else {
                    throw;
                }
            }

            read += 1;
            std::advance(it, 1);
            const auto direct = ReturnType::is_direct_push(opcode);

            if (direct.has_value()) {
                const auto& pushSize = direct.value();
                const auto remaining = target - read;
                const auto effectiveSize = allowInvalidOpcodes
                                               ? std::min(pushSize, remaining)
                                               : pushSize;

                if (((read + effectiveSize) > target)) {
                    logger("opentxs::factory::")(__func__)(
                        ": Incomplete direct data push")
                        .Flush();

                    return {};
                }

                data = space(effectiveSize);

                if (0u < effectiveSize) {
                    std::memcpy(data.value().data(), it, effectiveSize);
                }

                read += effectiveSize;
                std::advance(it, effectiveSize);

                continue;
            }

            const auto push = ReturnType::is_push(opcode);

            if (push.has_value()) {
                auto buf = be::little_uint32_buf_t{};

                {
                    const auto& sizeBytes = push.value();

                    OT_ASSERT(0 < sizeBytes);
                    OT_ASSERT(5 > sizeBytes);

                    const auto remaining = target - read;
                    const auto effectiveSize =
                        allowInvalidOpcodes ? std::min(sizeBytes, remaining)
                                            : sizeBytes;

                    if ((read + effectiveSize) > target) {
                        logger("opentxs::factory::")(__func__)(
                            ": Incomplete data push")
                            .Flush();

                        return {};
                    }

                    if (0u < effectiveSize) {
                        size = space(effectiveSize);
                        std::memcpy(size.value().data(), it, effectiveSize);
                        read += effectiveSize;
                        std::advance(it, effectiveSize);
                        std::memcpy(
                            static_cast<void*>(&buf),
                            size.value().data(),
                            effectiveSize);
                    }
                }

                {
                    const auto pushSize = std::size_t{buf.value()};
                    const auto remaining = target - read;
                    const auto effectiveSize =
                        allowInvalidOpcodes ? std::min(pushSize, remaining)
                                            : pushSize;

                    if ((read + effectiveSize) > target) {
                        logger("opentxs::factory::")(__func__)(
                            ": Data push bytes missing")
                            .Flush();

                        return {};
                    }

                    if (0u < effectiveSize) {
                        data = space(effectiveSize);
                        std::memcpy(data.value().data(), it, effectiveSize);
                    }

                    read += effectiveSize;
                    std::advance(it, effectiveSize);
                }

                continue;
            }
        }
    } catch (...) {
        logger("opentxs::factory::")(__func__)(": Unknown opcode").Flush();

        return {};
    }

    elements.shrink_to_fit();

    try {
        return std::make_unique<ReturnType>(
            chain, role, std::move(elements), bytes.size());
    } catch (const std::exception& e) {
        LogVerbose()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinScript(
    const blockchain::Type chain,
    blockchain::block::bitcoin::ScriptElements&& elements,
    const blockchain::block::bitcoin::Script::Position role) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Script>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Script;

    if (false == ReturnType::validate(elements)) {
        LogVerbose()("opentxs::factory::")(__func__)(": Invalid elements")
            .Flush();

        return {};
    }

    if ((0 == elements.size()) && (ReturnType::Position::Output == role)) {
        LogVerbose()("opentxs::factory::")(__func__)(": Empty input").Flush();

        return {};
    }

    elements.shrink_to_fit();

    return std::make_unique<ReturnType>(chain, role, std::move(elements));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::internal
{
auto Script::blank_signature(const blockchain::Type chain) noexcept
    -> const Space&
{
    static const auto output = space(72);

    return output;
}

auto Script::blank_pubkey(
    const blockchain::Type chain,
    const bool mode) noexcept -> const Space&
{
    static const auto compressed = space(33);
    static const auto uncompressed = space(65);

    return mode ? compressed : uncompressed;
}
}  // namespace opentxs::blockchain::block::bitcoin::internal

namespace opentxs::blockchain::block::bitcoin::implementation
{
Script::Script(
    const blockchain::Type chain,
    const Position role,
    ScriptElements&& elements,
    std::optional<std::size_t> size) noexcept
    : chain_(chain)
    , role_(role)
    , elements_(std::move(elements))
    , type_(get_type(role_, elements_))
    , size_(size)
{
}

Script::Script(const Script& rhs) noexcept
    : chain_(rhs.chain_)
    , role_(rhs.role_)
    , elements_(rhs.elements_)
    , type_(rhs.type_)
    , size_(rhs.size_)
{
}

auto Script::bytes(const value_type& element) noexcept -> std::size_t
{
    const auto& [opcode, invalid, bytes, data] = element;

    return (invalid.has_value() ? sizeof(invalid.value()) : sizeof(opcode)) +
           (bytes.has_value() ? bytes.value().size() : 0u) +
           (data.has_value() ? data.value().size() : 0u);
}

auto Script::bytes(const ScriptElements& script) noexcept -> std::size_t
{
    return std::accumulate(
        std::begin(script),
        std::end(script),
        std::size_t{0},
        [](const std::size_t& lhs, const value_type& rhs) -> std::size_t {
            return lhs + bytes(rhs);
        });
}

auto Script::CalculateHash160(
    const api::Session& api,
    const AllocateOutput output) const noexcept -> bool
{
    auto preimage = Space{};

    if (false == Serialize(writer(preimage))) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize script").Flush();

        return false;
    }

    return blockchain::ScriptHash(api, chain_, reader(preimage), output);
}

auto Script::CalculateSize() const noexcept -> std::size_t
{
    if (false == size_.has_value()) { size_ = bytes(elements_); }

    return size_.value();
}

auto Script::decode(const std::byte in) noexcept(false) -> OP
{
    static const auto map = robin_hood::unordered_flat_map<std::uint8_t, OP>{
        {static_cast<std::uint8_t>(OP::ZERO), OP::ZERO},
        {static_cast<std::uint8_t>(OP::PUSHDATA_1), OP::PUSHDATA_1},
        {static_cast<std::uint8_t>(OP::PUSHDATA_2), OP::PUSHDATA_2},
        {static_cast<std::uint8_t>(OP::PUSHDATA_3), OP::PUSHDATA_3},
        {static_cast<std::uint8_t>(OP::PUSHDATA_4), OP::PUSHDATA_4},
        {static_cast<std::uint8_t>(OP::PUSHDATA_5), OP::PUSHDATA_5},
        {static_cast<std::uint8_t>(OP::PUSHDATA_6), OP::PUSHDATA_6},
        {static_cast<std::uint8_t>(OP::PUSHDATA_7), OP::PUSHDATA_7},
        {static_cast<std::uint8_t>(OP::PUSHDATA_8), OP::PUSHDATA_8},
        {static_cast<std::uint8_t>(OP::PUSHDATA_9), OP::PUSHDATA_9},
        {static_cast<std::uint8_t>(OP::PUSHDATA_10), OP::PUSHDATA_10},
        {static_cast<std::uint8_t>(OP::PUSHDATA_11), OP::PUSHDATA_11},
        {static_cast<std::uint8_t>(OP::PUSHDATA_12), OP::PUSHDATA_12},
        {static_cast<std::uint8_t>(OP::PUSHDATA_13), OP::PUSHDATA_13},
        {static_cast<std::uint8_t>(OP::PUSHDATA_14), OP::PUSHDATA_14},
        {static_cast<std::uint8_t>(OP::PUSHDATA_15), OP::PUSHDATA_15},
        {static_cast<std::uint8_t>(OP::PUSHDATA_16), OP::PUSHDATA_16},
        {static_cast<std::uint8_t>(OP::PUSHDATA_17), OP::PUSHDATA_17},
        {static_cast<std::uint8_t>(OP::PUSHDATA_18), OP::PUSHDATA_18},
        {static_cast<std::uint8_t>(OP::PUSHDATA_19), OP::PUSHDATA_19},
        {static_cast<std::uint8_t>(OP::PUSHDATA_20), OP::PUSHDATA_20},
        {static_cast<std::uint8_t>(OP::PUSHDATA_21), OP::PUSHDATA_21},
        {static_cast<std::uint8_t>(OP::PUSHDATA_22), OP::PUSHDATA_22},
        {static_cast<std::uint8_t>(OP::PUSHDATA_23), OP::PUSHDATA_23},
        {static_cast<std::uint8_t>(OP::PUSHDATA_24), OP::PUSHDATA_24},
        {static_cast<std::uint8_t>(OP::PUSHDATA_25), OP::PUSHDATA_25},
        {static_cast<std::uint8_t>(OP::PUSHDATA_26), OP::PUSHDATA_26},
        {static_cast<std::uint8_t>(OP::PUSHDATA_27), OP::PUSHDATA_27},
        {static_cast<std::uint8_t>(OP::PUSHDATA_28), OP::PUSHDATA_28},
        {static_cast<std::uint8_t>(OP::PUSHDATA_29), OP::PUSHDATA_29},
        {static_cast<std::uint8_t>(OP::PUSHDATA_30), OP::PUSHDATA_30},
        {static_cast<std::uint8_t>(OP::PUSHDATA_31), OP::PUSHDATA_31},
        {static_cast<std::uint8_t>(OP::PUSHDATA_32), OP::PUSHDATA_32},
        {static_cast<std::uint8_t>(OP::PUSHDATA_33), OP::PUSHDATA_33},
        {static_cast<std::uint8_t>(OP::PUSHDATA_34), OP::PUSHDATA_34},
        {static_cast<std::uint8_t>(OP::PUSHDATA_35), OP::PUSHDATA_35},
        {static_cast<std::uint8_t>(OP::PUSHDATA_36), OP::PUSHDATA_36},
        {static_cast<std::uint8_t>(OP::PUSHDATA_37), OP::PUSHDATA_37},
        {static_cast<std::uint8_t>(OP::PUSHDATA_38), OP::PUSHDATA_38},
        {static_cast<std::uint8_t>(OP::PUSHDATA_39), OP::PUSHDATA_39},
        {static_cast<std::uint8_t>(OP::PUSHDATA_40), OP::PUSHDATA_40},
        {static_cast<std::uint8_t>(OP::PUSHDATA_41), OP::PUSHDATA_41},
        {static_cast<std::uint8_t>(OP::PUSHDATA_42), OP::PUSHDATA_42},
        {static_cast<std::uint8_t>(OP::PUSHDATA_43), OP::PUSHDATA_43},
        {static_cast<std::uint8_t>(OP::PUSHDATA_44), OP::PUSHDATA_44},
        {static_cast<std::uint8_t>(OP::PUSHDATA_45), OP::PUSHDATA_45},
        {static_cast<std::uint8_t>(OP::PUSHDATA_46), OP::PUSHDATA_46},
        {static_cast<std::uint8_t>(OP::PUSHDATA_47), OP::PUSHDATA_47},
        {static_cast<std::uint8_t>(OP::PUSHDATA_48), OP::PUSHDATA_48},
        {static_cast<std::uint8_t>(OP::PUSHDATA_49), OP::PUSHDATA_49},
        {static_cast<std::uint8_t>(OP::PUSHDATA_50), OP::PUSHDATA_50},
        {static_cast<std::uint8_t>(OP::PUSHDATA_51), OP::PUSHDATA_51},
        {static_cast<std::uint8_t>(OP::PUSHDATA_52), OP::PUSHDATA_52},
        {static_cast<std::uint8_t>(OP::PUSHDATA_53), OP::PUSHDATA_53},
        {static_cast<std::uint8_t>(OP::PUSHDATA_54), OP::PUSHDATA_54},
        {static_cast<std::uint8_t>(OP::PUSHDATA_55), OP::PUSHDATA_55},
        {static_cast<std::uint8_t>(OP::PUSHDATA_56), OP::PUSHDATA_56},
        {static_cast<std::uint8_t>(OP::PUSHDATA_57), OP::PUSHDATA_57},
        {static_cast<std::uint8_t>(OP::PUSHDATA_58), OP::PUSHDATA_58},
        {static_cast<std::uint8_t>(OP::PUSHDATA_59), OP::PUSHDATA_59},
        {static_cast<std::uint8_t>(OP::PUSHDATA_60), OP::PUSHDATA_60},
        {static_cast<std::uint8_t>(OP::PUSHDATA_61), OP::PUSHDATA_61},
        {static_cast<std::uint8_t>(OP::PUSHDATA_62), OP::PUSHDATA_62},
        {static_cast<std::uint8_t>(OP::PUSHDATA_63), OP::PUSHDATA_63},
        {static_cast<std::uint8_t>(OP::PUSHDATA_64), OP::PUSHDATA_64},
        {static_cast<std::uint8_t>(OP::PUSHDATA_65), OP::PUSHDATA_65},
        {static_cast<std::uint8_t>(OP::PUSHDATA_66), OP::PUSHDATA_66},
        {static_cast<std::uint8_t>(OP::PUSHDATA_67), OP::PUSHDATA_67},
        {static_cast<std::uint8_t>(OP::PUSHDATA_68), OP::PUSHDATA_68},
        {static_cast<std::uint8_t>(OP::PUSHDATA_69), OP::PUSHDATA_69},
        {static_cast<std::uint8_t>(OP::PUSHDATA_70), OP::PUSHDATA_70},
        {static_cast<std::uint8_t>(OP::PUSHDATA_71), OP::PUSHDATA_71},
        {static_cast<std::uint8_t>(OP::PUSHDATA_72), OP::PUSHDATA_72},
        {static_cast<std::uint8_t>(OP::PUSHDATA_73), OP::PUSHDATA_73},
        {static_cast<std::uint8_t>(OP::PUSHDATA_74), OP::PUSHDATA_74},
        {static_cast<std::uint8_t>(OP::PUSHDATA_75), OP::PUSHDATA_75},
        {static_cast<std::uint8_t>(OP::PUSHDATA1), OP::PUSHDATA1},
        {static_cast<std::uint8_t>(OP::PUSHDATA2), OP::PUSHDATA2},
        {static_cast<std::uint8_t>(OP::PUSHDATA4), OP::PUSHDATA4},
        {static_cast<std::uint8_t>(OP::ONE_NEGATE), OP::ONE_NEGATE},
        {static_cast<std::uint8_t>(OP::RESERVED), OP::RESERVED},
        {static_cast<std::uint8_t>(OP::ONE), OP::ONE},
        {static_cast<std::uint8_t>(OP::TWO), OP::TWO},
        {static_cast<std::uint8_t>(OP::THREE), OP::THREE},
        {static_cast<std::uint8_t>(OP::FOUR), OP::FOUR},
        {static_cast<std::uint8_t>(OP::FIVE), OP::FIVE},
        {static_cast<std::uint8_t>(OP::SIX), OP::SIX},
        {static_cast<std::uint8_t>(OP::SEVEN), OP::SEVEN},
        {static_cast<std::uint8_t>(OP::EIGHT), OP::EIGHT},
        {static_cast<std::uint8_t>(OP::NINE), OP::NINE},
        {static_cast<std::uint8_t>(OP::TEN), OP::TEN},
        {static_cast<std::uint8_t>(OP::ELEVEN), OP::ELEVEN},
        {static_cast<std::uint8_t>(OP::TWELVE), OP::TWELVE},
        {static_cast<std::uint8_t>(OP::THIRTEEN), OP::THIRTEEN},
        {static_cast<std::uint8_t>(OP::FOURTEEN), OP::FOURTEEN},
        {static_cast<std::uint8_t>(OP::FIFTEEN), OP::FIFTEEN},
        {static_cast<std::uint8_t>(OP::SIXTEEN), OP::SIXTEEN},
        {static_cast<std::uint8_t>(OP::NOP), OP::NOP},
        {static_cast<std::uint8_t>(OP::VER), OP::VER},
        {static_cast<std::uint8_t>(OP::IF), OP::IF},
        {static_cast<std::uint8_t>(OP::NOTIF), OP::NOTIF},
        {static_cast<std::uint8_t>(OP::VERIF), OP::VERIF},
        {static_cast<std::uint8_t>(OP::VERNOTIF), OP::VERNOTIF},
        {static_cast<std::uint8_t>(OP::ELSE), OP::ELSE},
        {static_cast<std::uint8_t>(OP::ENDIF), OP::ENDIF},
        {static_cast<std::uint8_t>(OP::VERIFY), OP::VERIFY},
        {static_cast<std::uint8_t>(OP::RETURN), OP::RETURN},
        {static_cast<std::uint8_t>(OP::TOALTSTACK), OP::TOALTSTACK},
        {static_cast<std::uint8_t>(OP::FROMALTSTACK), OP::FROMALTSTACK},
        {static_cast<std::uint8_t>(OP::TWO_DROP), OP::TWO_DROP},
        {static_cast<std::uint8_t>(OP::TWO_DUP), OP::TWO_DUP},
        {static_cast<std::uint8_t>(OP::THREE_DUP), OP::THREE_DUP},
        {static_cast<std::uint8_t>(OP::TWO_OVER), OP::TWO_OVER},
        {static_cast<std::uint8_t>(OP::TWO_ROT), OP::TWO_ROT},
        {static_cast<std::uint8_t>(OP::TWO_SWAP), OP::TWO_SWAP},
        {static_cast<std::uint8_t>(OP::IFDUP), OP::IFDUP},
        {static_cast<std::uint8_t>(OP::DEPTH), OP::DEPTH},
        {static_cast<std::uint8_t>(OP::DROP), OP::DROP},
        {static_cast<std::uint8_t>(OP::DUP), OP::DUP},
        {static_cast<std::uint8_t>(OP::NIP), OP::NIP},
        {static_cast<std::uint8_t>(OP::OVER), OP::OVER},
        {static_cast<std::uint8_t>(OP::PICK), OP::PICK},
        {static_cast<std::uint8_t>(OP::ROLL), OP::ROLL},
        {static_cast<std::uint8_t>(OP::ROT), OP::ROT},
        {static_cast<std::uint8_t>(OP::SWAP), OP::SWAP},
        {static_cast<std::uint8_t>(OP::TUCK), OP::TUCK},
        {static_cast<std::uint8_t>(OP::CAT), OP::CAT},
        {static_cast<std::uint8_t>(OP::SUBSTR), OP::SUBSTR},
        {static_cast<std::uint8_t>(OP::LEFT), OP::LEFT},
        {static_cast<std::uint8_t>(OP::RIGHT), OP::RIGHT},
        {static_cast<std::uint8_t>(OP::SIZE), OP::SIZE},
        {static_cast<std::uint8_t>(OP::INVERT), OP::INVERT},
        {static_cast<std::uint8_t>(OP::AND), OP::AND},
        {static_cast<std::uint8_t>(OP::OR), OP::OR},
        {static_cast<std::uint8_t>(OP::XOR), OP::XOR},
        {static_cast<std::uint8_t>(OP::EQUAL), OP::EQUAL},
        {static_cast<std::uint8_t>(OP::EQUALVERIFY), OP::EQUALVERIFY},
        {static_cast<std::uint8_t>(OP::RESERVED1), OP::RESERVED1},
        {static_cast<std::uint8_t>(OP::RESERVED2), OP::RESERVED2},
        {static_cast<std::uint8_t>(OP::ONE_ADD), OP::ONE_ADD},
        {static_cast<std::uint8_t>(OP::ONE_SUB), OP::ONE_SUB},
        {static_cast<std::uint8_t>(OP::TWO_MUL), OP::TWO_MUL},
        {static_cast<std::uint8_t>(OP::TWO_DIV), OP::TWO_DIV},
        {static_cast<std::uint8_t>(OP::NEGATE), OP::NEGATE},
        {static_cast<std::uint8_t>(OP::ABS), OP::ABS},
        {static_cast<std::uint8_t>(OP::NOT), OP::NOT},
        {static_cast<std::uint8_t>(OP::ZERO_NOTEQUAL), OP::ZERO_NOTEQUAL},
        {static_cast<std::uint8_t>(OP::ADD), OP::ADD},
        {static_cast<std::uint8_t>(OP::SUB), OP::SUB},
        {static_cast<std::uint8_t>(OP::MUL), OP::MUL},
        {static_cast<std::uint8_t>(OP::DIV), OP::DIV},
        {static_cast<std::uint8_t>(OP::MOD), OP::MOD},
        {static_cast<std::uint8_t>(OP::LSHIFT), OP::LSHIFT},
        {static_cast<std::uint8_t>(OP::RSHIFT), OP::RSHIFT},
        {static_cast<std::uint8_t>(OP::BOOLAND), OP::BOOLAND},
        {static_cast<std::uint8_t>(OP::BOOLOR), OP::BOOLOR},
        {static_cast<std::uint8_t>(OP::NUMEQUAL), OP::NUMEQUAL},
        {static_cast<std::uint8_t>(OP::NUMEQUALVERIFY), OP::NUMEQUALVERIFY},
        {static_cast<std::uint8_t>(OP::NUMNOTEQUAL), OP::NUMNOTEQUAL},
        {static_cast<std::uint8_t>(OP::LESSTHAN), OP::LESSTHAN},
        {static_cast<std::uint8_t>(OP::GREATERTHAN), OP::GREATERTHAN},
        {static_cast<std::uint8_t>(OP::LESSTHANOREQUAL), OP::LESSTHANOREQUAL},
        {static_cast<std::uint8_t>(OP::GREATERTHANOREQUAL),
         OP::GREATERTHANOREQUAL},
        {static_cast<std::uint8_t>(OP::MIN), OP::MIN},
        {static_cast<std::uint8_t>(OP::MAX), OP::MAX},
        {static_cast<std::uint8_t>(OP::WITHIN), OP::WITHIN},
        {static_cast<std::uint8_t>(OP::RIPEMD160), OP::RIPEMD160},
        {static_cast<std::uint8_t>(OP::SHA1), OP::SHA1},
        {static_cast<std::uint8_t>(OP::SHA256), OP::SHA256},
        {static_cast<std::uint8_t>(OP::HASH160), OP::HASH160},
        {static_cast<std::uint8_t>(OP::HASH256), OP::HASH256},
        {static_cast<std::uint8_t>(OP::CODESEPARATOR), OP::CODESEPARATOR},
        {static_cast<std::uint8_t>(OP::CHECKSIG), OP::CHECKSIG},
        {static_cast<std::uint8_t>(OP::CHECKSIGVERIFY), OP::CHECKSIGVERIFY},
        {static_cast<std::uint8_t>(OP::CHECKMULTISIG), OP::CHECKMULTISIG},
        {static_cast<std::uint8_t>(OP::CHECKMULTISIGVERIFY),
         OP::CHECKMULTISIGVERIFY},
        {static_cast<std::uint8_t>(OP::NOP1), OP::NOP1},
        {static_cast<std::uint8_t>(OP::NOP2), OP::NOP2},
        {static_cast<std::uint8_t>(OP::NOP3), OP::NOP3},
        {static_cast<std::uint8_t>(OP::NOP4), OP::NOP4},
        {static_cast<std::uint8_t>(OP::NOP5), OP::NOP5},
        {static_cast<std::uint8_t>(OP::NOP6), OP::NOP6},
        {static_cast<std::uint8_t>(OP::NOP7), OP::NOP7},
        {static_cast<std::uint8_t>(OP::NOP8), OP::NOP8},
        {static_cast<std::uint8_t>(OP::NOP9), OP::NOP9},
        {static_cast<std::uint8_t>(OP::NOP10), OP::NOP10},
        {static_cast<std::uint8_t>(OP::PUBKEYHASH), OP::PUBKEYHASH},
        {static_cast<std::uint8_t>(OP::PUBKEY), OP::PUBKEY},
        {static_cast<std::uint8_t>(OP::INVALIDOPCODE), OP::INVALIDOPCODE},
    };

    return map.at(std::to_integer<std::uint8_t>(in));
}

auto Script::evaluate_data(const ScriptElements& script) noexcept -> Pattern
{
    OT_ASSERT(2 <= script.size());

    for (auto i = std::size_t{1}; i < script.size(); ++i) {
        if (false == is_data_push(script.at(i))) { return Pattern::Custom; }
    }

    return Pattern::NullData;
}

auto Script::evaluate_multisig(const ScriptElements& script) noexcept -> Pattern
{
    OT_ASSERT(4 <= script.size());

    auto it = script.crbegin();
    std::advance(it, 1);
    const auto n = to_number(it->opcode_);

    if (0u == n) { return Pattern::Malformed; }
    if ((3u + n) > script.size()) { return Pattern::Malformed; }

    for (auto i = std::uint8_t{0}; i < n; ++i) {
        std::advance(it, 1);

        if (false == is_public_key(*it)) { return Pattern::Malformed; }
    }

    std::advance(it, 1);
    const auto m = to_number(it->opcode_);

    if ((0u == m) || (m > n)) { return Pattern::Malformed; }

    if ((3u + n) < script.size()) { return Pattern::Custom; }

    return Pattern::PayToMultisig;
}

auto Script::evaluate_pubkey(const ScriptElements& script) noexcept -> Pattern
{
    OT_ASSERT(2 == script.size());

    if (is_public_key(script.at(0))) { return Pattern::PayToPubkey; }

    return Pattern::Custom;
}

auto Script::evaluate_pubkey_hash(const ScriptElements& script) noexcept
    -> Pattern
{
    OT_ASSERT(5 == script.size());

    auto it = script.cbegin();

    if (OP::DUP != it->opcode_) { return Pattern::Custom; }

    std::advance(it, 1);

    if (OP::HASH160 != it->opcode_) { return Pattern::Custom; }

    std::advance(it, 1);

    if (false == is_hash160(*it)) { return Pattern::Custom; }

    std::advance(it, 1);

    if (OP::EQUALVERIFY != it->opcode_) { return Pattern::Custom; }

    return Pattern::PayToPubkeyHash;
}

auto Script::evaluate_script_hash(const ScriptElements& script) noexcept
    -> Pattern
{
    OT_ASSERT(3 == script.size());

    auto it = script.cbegin();

    if (OP::HASH160 != it->opcode_) { return Pattern::Custom; }

    std::advance(it, 1);

    if (false == is_hash160(*it)) { return Pattern::Custom; }
    if (OP::PUSHDATA_20 != it->opcode_) { return Pattern::Custom; }

    return Pattern::PayToScriptHash;
}

auto Script::evaluate_segwit(const ScriptElements& script) noexcept -> Pattern
{
    OT_ASSERT(2 == script.size());

    const auto& opcode = script.at(0).opcode_;
    const auto& program = script.at(1).data_.value();

    switch (to_number(opcode)) {
        case 0: {
            switch (program.size()) {
                case 20: {
                    return Pattern::PayToWitnessPubkeyHash;
                }
                case 32: {
                    return Pattern::PayToWitnessScriptHash;
                }
                default: {
                }
            }
        } break;
        case 1: {
            switch (program.size()) {
                case 32: {
                    return Pattern::PayToTaproot;
                }
                default: {
                }
            }
        } break;
        default: {
        }
    }

    return Pattern::Custom;
}

auto Script::ExtractElements(const cfilter::Type style) const noexcept
    -> Vector<Vector<std::byte>>
{
    if (0 == elements_.size()) {
        LogTrace()(OT_PRETTY_CLASS())("skipping empty script").Flush();

        return {};
    }

    auto output = Vector<Vector<std::byte>>{};

    switch (style) {
        case cfilter::Type::ES: {
            LogTrace()(OT_PRETTY_CLASS())("processing data pushes").Flush();

            for (const auto& element : *this) {
                if (is_data_push(element)) {
                    const auto& data = element.data_.value();
                    auto it{data.data()};

                    switch (data.size()) {
                        case 65: {
                            std::advance(it, 1);
                            [[fallthrough]];
                        }
                        case 64: {
                            output.emplace_back(it, it + 32);
                            std::advance(it, 32);
                            output.emplace_back(it, it + 32);
                            [[fallthrough]];
                        }
                        case 33:
                        case 32:
                        case 20: {
                            output.emplace_back(data.cbegin(), data.cend());
                        } break;
                        default: {
                        }
                    }
                }
            }

            if (const auto subscript = RedeemScript(); subscript) {
                auto temp = subscript->ExtractElements(style);
                output.insert(
                    output.end(),
                    std::make_move_iterator(temp.begin()),
                    std::make_move_iterator(temp.end()));
            }
        } break;
        case cfilter::Type::Basic_BIP158:
        case cfilter::Type::Basic_BCHVariant:
        default: {
            if (OP::RETURN == elements_.at(0).opcode_) {
                LogTrace()(OT_PRETTY_CLASS())("skipping null data script")
                    .Flush();

                return {};
            }

            LogTrace()(OT_PRETTY_CLASS())("processing serialized script")
                .Flush();
            auto& script = output.emplace_back();
            Serialize(writer(script));
        }
    }

    LogTrace()(OT_PRETTY_CLASS())("extracted ")(output.size())(" elements")
        .Flush();
    std::sort(output.begin(), output.end());

    return output;
}

auto Script::ExtractPatterns(const api::Session& api) const noexcept
    -> UnallocatedVector<PatternID>
{
    auto output = UnallocatedVector<PatternID>{};
    const auto hashes = LikelyPubkeyHashes(api);
    std::transform(
        std::begin(hashes),
        std::end(hashes),
        std::back_inserter(output),
        [&](const auto& hash) -> auto {
            return api.Crypto().Blockchain().IndexItem(hash->Bytes());
        });

    return output;
}

auto Script::first_opcode(const ScriptElements& script) noexcept -> OP
{
    return script.cbegin()->opcode_;
}

auto Script::get_data(const std::size_t position) const noexcept(false)
    -> ReadView
{
    auto& data = elements_.at(position).data_;

    if (false == data.has_value()) {
        throw std::out_of_range("No data at specified script position");
    }

    return reader(data.value());
}

auto Script::get_opcode(const std::size_t position) const noexcept(false) -> OP
{
    return elements_.at(position).opcode_;
}

auto Script::get_type(
    const Position role,
    const ScriptElements& script) noexcept -> Pattern
{
    if (0 == script.size()) { return Pattern::Empty; }

    switch (role) {
        case Position::Coinbase: {

            return Pattern::Coinbase;
        }
        case Position::Input: {

            return Pattern::Input;
        }
        case Position::Redeem:
        case Position::Output: {
            if (potential_pubkey_hash(script)) {
                return evaluate_pubkey_hash(script);
            } else if (potential_script_hash(script)) {
                return evaluate_script_hash(script);
            } else if (potential_data(script)) {
                return evaluate_data(script);
            } else if (potential_segwit(script)) {
                return evaluate_segwit(script);
            } else if (potential_pubkey(script)) {
                return evaluate_pubkey(script);
            } else if (potential_multisig(script)) {
                return evaluate_multisig(script);
            } else {
                return Pattern::Custom;
            }
        }
        default: {
            OT_FAIL;
        }
    }
}

auto Script::IsNotification(
    const std::uint8_t version,
    const PaymentCode& recipient) const noexcept -> bool
{
    if (Pattern::PayToMultisig != Type()) { return false; }

    if (M().value_or(0) != 1u) { return false; }
    if (N().value_or(0) != 3u) { return false; }

    const auto key = MultisigPubkey(1);

    if (false == key.has_value()) { return false; }

    const auto bytes = key.value();

    if ((33u != bytes.size()) && (65u != bytes.size())) { return false; }

    const auto expect = [&] {
        auto out = Space{};
        recipient.Locator(writer(out), version);

        return out;
    }();

    if (32u != expect.size()) { return false; }

    return 0 == std::memcmp(expect.data(), std::next(bytes.data()), 32);
}

auto Script::is_data_push(const value_type& element) noexcept -> bool
{
    return validate(element, true);
}

auto Script::is_direct_push(const OP opcode) noexcept(false)
    -> std::optional<std::size_t>
{
    const auto value = static_cast<std::uint8_t>(opcode);

    if ((0 < value) && (76 > value)) { return value; }

    return std::nullopt;
}

auto Script::is_push(const OP opcode) noexcept(false)
    -> std::optional<std::size_t>
{
    constexpr auto low = std::size_t{75u};
    constexpr auto high = std::size_t{79u};
    constexpr auto shift = std::size_t{low + 1u};
    constexpr auto one = std::size_t{1};
    const auto value = std::size_t{static_cast<std::uint8_t>(opcode)};

    if ((low < value) && (high > value)) {
        return std::size_t{one << (value - shift)};
    }

    return std::nullopt;
}

auto Script::is_hash160(const value_type& element) noexcept -> bool
{
    if (false == is_data_push(element)) { return false; }

    return 20 == element.data_->size();
}

auto Script::is_public_key(const value_type& element) noexcept -> bool
{
    if (false == is_data_push(element)) { return false; }

    const auto size = element.data_->size();

    return (33 == size) || (65 == size);
}

auto Script::last_opcode(const ScriptElements& script) noexcept -> OP
{
    return script.crbegin()->opcode_;
}

auto Script::LikelyPubkeyHashes(const api::Session& api) const noexcept
    -> UnallocatedVector<OTData>
{
    auto output = UnallocatedVector<OTData>{};

    switch (type_) {
        case Pattern::PayToPubkeyHash: {
            const auto hash = PubkeyHash();

            OT_ASSERT(hash.has_value());

            output.emplace_back(api.Factory().DataFromBytes(hash.value()));
        } break;
        case Pattern::PayToMultisig: {
            for (auto i = std::uint8_t{0}; i < N().value(); ++i) {
                auto hash = api.Factory().Data();
                const auto key = MultisigPubkey(i);

                OT_ASSERT(key.has_value());

                blockchain::PubkeyHash(
                    api, chain_, key.value(), hash->WriteInto());
                output.emplace_back(std::move(hash));
            }
        } break;
        case Pattern::PayToPubkey: {
            auto hash = api.Factory().Data();
            const auto key = Pubkey();

            OT_ASSERT(key.has_value());

            blockchain::PubkeyHash(api, chain_, key.value(), hash->WriteInto());
            output.emplace_back(std::move(hash));
        } break;
        case Pattern::Coinbase:
        case Pattern::PayToScriptHash:
        case Pattern::Empty:
        case Pattern::Malformed: {
        } break;
        case Pattern::Custom:
        case Pattern::NullData:
        case Pattern::Input:
        default: {
            for (const auto& element : elements_) {
                if (is_hash160(element)) {
                    OT_ASSERT(element.data_.has_value());

                    output.emplace_back(api.Factory().DataFromBytes(
                        reader(element.data_.value())));
                } else if (is_public_key(element)) {
                    OT_ASSERT(element.data_.has_value());

                    auto hash = api.Factory().Data();
                    blockchain::PubkeyHash(
                        api,
                        chain_,
                        reader(element.data_.value()),
                        hash->WriteInto());
                    output.emplace_back(std::move(hash));
                }
            }
        }
    }

    return output;
}

auto Script::M() const noexcept -> std::optional<std::uint8_t>
{
    if (Pattern::PayToMultisig != type_) { return {}; }

    return to_number(get_opcode(0));
}

auto Script::MultisigPubkey(const std::size_t position) const noexcept
    -> std::optional<ReadView>
{
    if (Pattern::PayToMultisig != type_) { return {}; }

    const auto index = std::size_t{position + 1u};

    if (index > N()) { return {}; }

    return get_data(index);
}

auto Script::N() const noexcept -> std::optional<std::uint8_t>
{
    if (Pattern::PayToMultisig != type_) { return {}; }

    return to_number(get_opcode(elements_.size() - 2));
}

auto Script::potential_data(const ScriptElements& script) noexcept -> bool
{
    return (OP::RETURN == first_opcode(script)) && (2 <= script.size());
}

auto Script::potential_multisig(const ScriptElements& script) noexcept -> bool
{
    return (OP::CHECKMULTISIG == last_opcode(script)) && (4 <= script.size());
}

auto Script::potential_pubkey(const ScriptElements& script) noexcept -> bool
{
    return (OP::CHECKSIG == last_opcode(script)) && (2 == script.size());
}

auto Script::potential_pubkey_hash(const ScriptElements& script) noexcept
    -> bool
{
    return (OP::CHECKSIG == last_opcode(script)) && (5 == script.size());
}

auto Script::potential_script_hash(const ScriptElements& script) noexcept
    -> bool
{
    return (OP::HASH160 == first_opcode(script)) &&
           (OP::EQUAL == last_opcode(script)) && (3 == script.size());
}

auto Script::potential_segwit(const ScriptElements& script) noexcept -> bool
{
    if (2 != script.size()) { return false; }

    switch (first_opcode(script)) {
        case OP::ZERO:
        case OP::ONE:
        case OP::TWO:
        case OP::THREE:
        case OP::FOUR:
        case OP::FIVE:
        case OP::SIX:
        case OP::SEVEN:
        case OP::EIGHT:
        case OP::NINE:
        case OP::TEN:
        case OP::ELEVEN:
        case OP::TWELVE:
        case OP::THIRTEEN:
        case OP::FOURTEEN:
        case OP::FIFTEEN:
        case OP::SIXTEEN: {
        } break;
        default: {

            return false;
        }
    }

    const auto& element = script.at(1);

    if (false == element.data_.has_value()) { return false; }

    const auto& program = element.data_.value();

    return (1u < program.size()) && (41u > program.size());
}

auto Script::Print() const noexcept -> UnallocatedCString
{
    auto output = std::stringstream{};

    for (const auto& [opcode, invalid, push, data] : elements_) {
        output << "      op: "
               << std::to_string(static_cast<std::uint8_t>(opcode));

        if (invalid) {
            output << " invalid: "
                   << std::to_string(
                          std::to_integer<std::uint8_t>(invalid.value()));
        }

        if (push) {
            auto bytes = std::uint64_t{};
            std::memcpy(
                &bytes,
                push.value().data(),
                std::min(push.value().size(), sizeof(bytes)));
            output << " push bytes: " << bytes;
        }

        if (data) {
            auto item = Data::Factory();
            item->Assign(reader(data.value()));
            output << " (" << item->size() << ") bytes : " << item->asHex();
        }

        output << '\n';
    }

    return output.str();
}

auto Script::Pubkey() const noexcept -> std::optional<ReadView>
{
    switch (type_) {
        case Pattern::PayToPubkey: {
            return get_data(0);
        }
        case Pattern::PayToTaproot: {
            return get_data(1);
        }
        default: {
            return {};
        }
    }
}

auto Script::PubkeyHash() const noexcept -> std::optional<ReadView>
{
    switch (type_) {
        case Pattern::PayToPubkeyHash: {
            return get_data(2);
        }
        case Pattern::PayToWitnessPubkeyHash: {
            return get_data(1);
        }
        default: {
            return {};
        }
    }
}

auto Script::RedeemScript() const noexcept -> std::unique_ptr<bitcoin::Script>
{
    if (Position::Input != role_) { return {}; }
    if (0 == elements_.size()) { return {}; }

    const auto& element = *elements_.crbegin();

    if (false == is_data_push(element)) { return {}; }

    return factory::BitcoinScript(
        chain_, reader(element.data_.value()), Position::Redeem, true, true);
}

auto Script::ScriptHash() const noexcept -> std::optional<ReadView>
{
    switch (type_) {
        case Pattern::PayToScriptHash:
        case Pattern::PayToWitnessScriptHash: {
            return get_data(1);
        }
        default: {
            return {};
        }
    }
}

auto Script::Serialize(const AllocateOutput destination) const noexcept -> bool
{
    if (!destination) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return false;
    }

    const auto size = CalculateSize();

    if (0 == size) { return true; }

    auto output = destination(size);

    if (false == output.valid(size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate output bytes")
            .Flush();

        return false;
    }

    auto it = static_cast<std::byte*>(output.data());

    for (const auto& element : elements_) {
        const auto& [opcode, invalid, bytes, data] = element;

        if (invalid.has_value()) {
            const auto& code = invalid.value();
            std::memcpy(static_cast<void*>(it), &code, sizeof(code));
            std::advance(it, sizeof(code));
        } else {
            std::memcpy(static_cast<void*>(it), &opcode, sizeof(opcode));
            std::advance(it, sizeof(opcode));
        }

        if (bytes.has_value()) {
            const auto& value = bytes.value();
            std::memcpy(static_cast<void*>(it), value.data(), value.size());
            std::advance(it, value.size());
        }

        if (data.has_value()) {
            const auto& value = data.value();

            if (0u < value.size()) {
                std::memcpy(static_cast<void*>(it), value.data(), value.size());
            }

            std::advance(it, value.size());
        }
    }

    return true;
}

auto Script::SigningSubscript(const blockchain::Type chain) const noexcept
    -> std::unique_ptr<internal::Script>
{
    switch (type_) {
        case Pattern::PayToWitnessPubkeyHash: {
            auto elements = [&] {
                auto out = ScriptElements{};
                out.emplace_back(internal::Opcode(OP::DUP));
                out.emplace_back(internal::Opcode(OP::HASH160));
                out.emplace_back(internal::PushData(PubkeyHash().value()));
                out.emplace_back(internal::Opcode(OP::EQUALVERIFY));
                out.emplace_back(internal::Opcode(OP::CHECKSIG));

                return out;
            }();

            return std::make_unique<Script>(
                chain_, Position::Output, std::move(elements));
        }
        default: {
            // TODO handle OP_CODESEPERATOR shit

            return clone();
        }
    }
}

auto Script::to_number(const OP opcode) noexcept -> std::uint8_t
{
    if ((OP::ONE <= opcode) && (OP::SIXTEEN >= opcode)) {

        return static_cast<std::uint8_t>(opcode) -
               static_cast<std::uint8_t>(OP::ONE) + 1u;
    }

    return 0;
}

auto Script::validate(const ScriptElements& elements) noexcept -> bool
{
    for (const auto& element : elements) {
        if (false == validate(element, false)) { return false; }
    }

    return true;
}

auto Script::validate(
    const ScriptElement& element,
    const bool checkForData) noexcept -> bool
{
    const auto& [opcode, invalid, bytes, data] = element;

    if (invalid.has_value()) { return !checkForData; }

    if (auto size = is_direct_push(opcode); size.has_value()) {
        if (bytes.has_value()) { return false; }
        if (false == data.has_value()) { return false; }
        if (size != data.value().size()) { return false; }

        return true;
    } else if (auto push = is_push(opcode); push.has_value()) {
        if (false == bytes.has_value()) { return false; }
        if (false == data.has_value()) { return false; }

        auto size = std::size_t{};
        const auto& pushBytes = push.value();

        if (pushBytes != bytes->size()) { return false; }

        switch (pushBytes) {
            case 1:
            case 2:
            case 4: {
                auto buf = be::little_uint32_buf_t{};
                std::memcpy(
                    static_cast<void*>(&buf), bytes.value().data(), pushBytes);
                size = buf.value();
            } break;
            default: {
                OT_FAIL;
            }
        }

        if (size != data.value().size()) { return false; }

        return true;
    } else {

        return !checkForData;
    }
}

auto Script::Value(const std::size_t position) const noexcept
    -> std::optional<ReadView>
{
    if (Pattern::NullData != type_) { return {}; }

    const auto index = position + 1u;

    if (index > elements_.size()) { return {}; }

    return get_data(index);
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation

// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/block/bitcoin/Input.hpp"  // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iterator>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "blockchain/bitcoin/CompactSize.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/BlockchainInputWitness.pb.h"
#include "opentxs/protobuf/BlockchainPreviousOutput.pb.h"
#include "opentxs/protobuf/BlockchainTransactionInput.pb.h"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"
#include "opentxs/protobuf/BlockchainWalletKey.pb.h"

namespace be = boost::endian;

#define OT_METHOD "opentxs::blockchain::block::bitcoin::implementation::Input::"

namespace opentxs::factory
{
using ReturnType = blockchain::block::bitcoin::implementation::Input;

auto BitcoinTransactionInput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const UTXO& spends,
    const std::optional<std::uint32_t> sequence) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>
{
    namespace b = opentxs::blockchain;
    namespace bb = b::block::bitcoin;
    namespace bi = bb::internal;

    auto pPrevOut =
        BitcoinTransactionOutput(api, blockchain, chain, spends.second);

    if (false == bool(pPrevOut)) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(
            ": Failed to instantiate previous output")
            .Flush();

        return {};
    }

    const auto& prevOut = *pPrevOut;
    const auto& outputKeys = prevOut.Keys();

    if (0 == outputKeys.size()) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(
            ": No keys associated with previous output")
            .Flush();

        return {};
    }

    // TODO if this is input spends a segwit script then make a dummy witness
    auto elements = bb::ScriptElements{};

    switch (prevOut.Script().Type()) {
        case bb::Script::Pattern::PayToPubkeyHash: {
            // TODO verify the hash is the hash of the uncompressed key
            elements.emplace_back(bb::internal::PushData(
                reader(bi::Script::blank_signature(chain))));
            elements.emplace_back(bb::internal::PushData(
                reader(bi::Script::blank_pubkey(chain))));
        } break;
        case bb::Script::Pattern::PayToPubkey: {
            elements.emplace_back(bb::internal::PushData(
                reader(bi::Script::blank_signature(chain))));
        } break;
        case bb::Script::Pattern::PayToMultisig: {
            // TODO this is probably wrong. Come up with a better algorithm once
            // multisig is supported
            const auto n = outputKeys.size();

            for (auto i = std::uint8_t{0}; i < n; ++i) {
                elements.emplace_back(bb::internal::PushData(
                    reader(bi::Script::blank_signature(chain))));
            }
        } break;
        default: {
            LogOutput("opentxs::factory::")(__FUNCTION__)(
                ": Unhandled script type")
                .Flush();

            return {};
        }
    }

    auto keys = boost::container::flat_set<ReturnType::KeyID>{};
    std::for_each(
        std::begin(outputKeys), std::end(outputKeys), [&](const auto& key) {
            keys.emplace(key);
        });

    return std::make_unique<ReturnType>(
        api,
        blockchain,
        chain,
        sequence.value_or(0xffffffff),
        blockchain::block::bitcoin::Outpoint{spends.first},
        std::vector<Space>{},
        BitcoinScript(chain, std::move(elements)),
        ReturnType::default_version_,
        std::move(pPrevOut),
        std::move(keys));
}

auto BitcoinTransactionInput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const ReadView outpoint,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool coinbase,
    std::vector<Space>&& witness) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>
{
    try {
        auto buf = be::little_uint32_buf_t{};

        if (sequence.size() != sizeof(buf)) {
            throw std::runtime_error("Invalid sequence");
        }

        std::memcpy(static_cast<void*>(&buf), sequence.data(), sequence.size());

        if (coinbase) {
            return std::make_unique<ReturnType>(
                api,
                blockchain,
                chain,
                buf.value(),
                blockchain::block::bitcoin::Outpoint{outpoint},
                std::move(witness),
                script,
                ReturnType::default_version_,
                nullptr,
                outpoint.size() + cs.Total() + sequence.size());
        } else {
            return std::make_unique<ReturnType>(
                api,
                blockchain,
                chain,
                buf.value(),
                blockchain::block::bitcoin::Outpoint{outpoint},
                std::move(witness),
                factory::BitcoinScript(chain, script, false, false),
                ReturnType::default_version_,
                outpoint.size() + cs.Total() + sequence.size());
        }
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransactionInput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const proto::BlockchainTransactionInput in,
    const bool coinbase) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>
{
    const auto& outpoint = in.previous();
    auto witness = std::vector<Space>{};

    for (const auto& bytes : in.witness().item()) {
        const auto it = reinterpret_cast<const std::byte*>(bytes.data());
        witness.emplace_back(it, it + bytes.size());
    }

    auto spends =
        std::unique_ptr<blockchain::block::bitcoin::internal::Output>{};

    if (in.has_spends()) {
        spends = factory::BitcoinTransactionOutput(
            api, blockchain, chain, in.spends());
    }

    try {
        if (coinbase) {
            return std::make_unique<ReturnType>(
                api,
                blockchain,
                chain,
                in.sequence(),
                blockchain::block::bitcoin::Outpoint{
                    outpoint.txid(),
                    static_cast<std::uint32_t>(outpoint.index())},
                std::move(witness),
                in.script(),
                in.version(),
                std::move(spends));
        } else {
            auto keys = boost::container::flat_set<ReturnType::KeyID>{};
            auto pkh = boost::container::flat_set<blockchain::PatternID>{};

            for (const auto& key : in.key()) {
                keys.emplace(
                    key.subaccount(),
                    static_cast<api::client::blockchain::Subchain>(
                        static_cast<std::uint8_t>(key.subchain())),
                    key.index());
            }

            for (const auto& pattern : in.pubkey_hash()) {
                pkh.emplace(pattern);
            }

            return std::make_unique<ReturnType>(
                api,
                blockchain,
                chain,
                in.sequence(),
                blockchain::block::bitcoin::Outpoint{
                    outpoint.txid(),
                    static_cast<std::uint32_t>(outpoint.index())},
                std::move(witness),
                factory::BitcoinScript(chain, in.script(), false, coinbase),
                Space{},
                in.version(),
                std::nullopt,
                std::move(keys),
                std::move(pkh),
                (in.has_script_hash()
                     ? std::make_optional<blockchain::PatternID>(
                           in.script_hash())
                     : std::nullopt),
                in.indexed(),
                std::move(spends));
        }
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin
{
Outpoint::Outpoint() noexcept
    : txid_()
    , index_()
{
}
Outpoint::Outpoint(const Outpoint& rhs) noexcept
    : txid_(rhs.txid_)
    , index_(rhs.index_)
{
}
Outpoint::Outpoint(Outpoint&& rhs) noexcept
    : Outpoint(rhs)  // copy constructor, rhs is an lvalue
{
}
Outpoint::Outpoint(const ReadView in) noexcept(false)
    : txid_()
    , index_()
{
    if (in.size() < sizeof(*this)) {
        throw std::runtime_error("Invalid bytes");
    }

    std::memcpy(static_cast<void*>(this), in.data(), sizeof(*this));
}
Outpoint::Outpoint(const ReadView txid, const std::uint32_t index) noexcept(
    false)
    : txid_()
    , index_()
{
    if (txid_.size() != txid.size()) {
        throw std::runtime_error("Invalid txid");
    }

    const auto buf = be::little_uint32_buf_t{index};

    static_assert(sizeof(index_) == sizeof(buf));

    std::memcpy(static_cast<void*>(txid_.data()), txid.data(), txid_.size());
    std::memcpy(static_cast<void*>(index_.data()), &buf, index_.size());
}
Outpoint& Outpoint::operator=(const Outpoint& rhs) noexcept
{
    if (&rhs != this) {
        txid_ = rhs.txid_;
        index_ = rhs.index_;
    }

    return *this;
}
Outpoint& Outpoint::operator=(Outpoint&& rhs) noexcept
{
    return operator=(rhs);  // copy assignment, rhs is an lvalue
}
auto Outpoint::Bytes() const noexcept -> ReadView
{
    return ReadView{reinterpret_cast<const char*>(this), sizeof(*this)};
}

auto Outpoint::operator<(const Outpoint& rhs) const noexcept -> bool
{
    return 0 > std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::operator<=(const Outpoint& rhs) const noexcept -> bool
{
    return 0 >= std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::operator>(const Outpoint& rhs) const noexcept -> bool
{
    return 0 < std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::operator>=(const Outpoint& rhs) const noexcept -> bool
{
    return 0 <= std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::operator==(const Outpoint& rhs) const noexcept -> bool
{
    return 0 == std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::operator!=(const Outpoint& rhs) const noexcept -> bool
{
    return 0 != std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::Index() const noexcept -> std::uint32_t
{
    auto buf = be::little_uint32_buf_t{};

    static_assert(sizeof(index_) == sizeof(buf));

    std::memcpy(static_cast<void*>(&buf), index_.data(), index_.size());

    return buf.value();
}

auto Outpoint::str() const noexcept -> std::string
{
    auto out = std::stringstream{};

    for (const auto byte : txid_) {
        out << std::hex << std::setfill('0') << std::setw(2)
            << std::to_integer<int>(byte);
    }

    out << ':' << Index();

    return out.str();
}

auto Outpoint::Txid() const noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(txid_.data()), txid_.size()};
}
}  // namespace opentxs::blockchain::block::bitcoin

namespace opentxs::blockchain::block::bitcoin::implementation
{
const VersionNumber Input::default_version_{1};
const VersionNumber Input::outpoint_version_{1};
const VersionNumber Input::key_version_{1};

Input::Input(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::vector<Space>&& witness,
    std::unique_ptr<const internal::Script> script,
    Space&& coinbase,
    const VersionNumber version,
    std::optional<std::size_t> size,
    boost::container::flat_set<KeyID>&& keys,
    boost::container::flat_set<PatternID>&& pubkeyHashes,
    std::optional<PatternID>&& scriptHash,
    const bool indexed,
    std::unique_ptr<const internal::Output> output) noexcept(false)
    : api_(api)
    , chain_(chain)
    , serialize_version_(version)
    , previous_(std::move(previous))
    , witness_(std::move(witness))
    , script_(std::move(script))
    , coinbase_(std::move(coinbase))
    , sequence_(sequence)
    , pubkey_hashes_(std::move(pubkeyHashes))
    , script_hash_(std::move(scriptHash))
    , previous_output_(std::move(output))
    , size_(size)
    , normalized_size_()
    , keys_(std::move(keys))
{
    if (false == bool(script_)) {
        throw std::runtime_error("Invalid input script");
    }

    if ((0 < coinbase_.size()) && (0 < script_->size())) {
        throw std::runtime_error("Input has both script and coinbase");
    }

    if (false == indexed) { index_elements(blockchain); }
}

Input::Input(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::vector<Space>&& witness,
    std::unique_ptr<const internal::Script> script,
    const VersionNumber version,
    std::optional<std::size_t> size) noexcept(false)
    : Input(
          api,
          blockchain,
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          std::move(script),
          Space{},
          version,
          size,
          {},
          {},
          {},
          false,
          nullptr)
{
}

Input::Input(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::vector<Space>&& witness,
    std::unique_ptr<const internal::Script> script,
    const VersionNumber version,
    std::unique_ptr<const internal::Output> output,
    boost::container::flat_set<KeyID>&& keys) noexcept(false)
    : Input(
          api,
          blockchain,
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          std::move(script),
          Space{},
          version,
          {},
          std::move(keys),
          {},
          {},
          false,
          std::move(output))
{
}

Input::Input(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::vector<Space>&& witness,
    const ReadView coinbase,
    const VersionNumber version,
    std::unique_ptr<const internal::Output> output,
    std::optional<std::size_t> size) noexcept(false)
    : Input(
          api,
          blockchain,
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          factory::BitcoinScript(chain, ScriptElements{}, false, true),
          Space{
              reinterpret_cast<const std::byte*>(coinbase.data()),
              reinterpret_cast<const std::byte*>(coinbase.data()) +
                  coinbase.size()},
          version,
          size,
          {},
          {},
          {},
          false,
          std::move(output))
{
}

Input::Input(const Input& rhs) noexcept
    : Input(rhs, rhs.script_->clone())
{
}

Input::Input(
    const Input& rhs,
    std::unique_ptr<const internal::Script> script) noexcept
    : api_(rhs.api_)
    , chain_(rhs.chain_)
    , serialize_version_(rhs.serialize_version_)
    , previous_(rhs.previous_)
    , witness_(rhs.witness_.begin(), rhs.witness_.end())
    , script_(std::move(script))
    , coinbase_(rhs.coinbase_.begin(), rhs.coinbase_.end())
    , sequence_(rhs.sequence_)
    , pubkey_hashes_(rhs.pubkey_hashes_)
    , script_hash_(rhs.script_hash_)
    , previous_output_(
          rhs.previous_output_ ? rhs.previous_output_->clone() : nullptr)
    , size_()
    , normalized_size_()
    , keys_(rhs.keys_)
{
}

auto Input::AddSignatures(const Signatures& signatures) noexcept -> bool
{
    // TODO add to witness if segwit

    auto elements = ScriptElements{};

    for (const auto& [lhs, rhs] : signatures) {
        elements.emplace_back(internal::PushData(lhs));
        elements.emplace_back(internal::PushData(rhs));
    }

    auto& script =
        const_cast<std::unique_ptr<const internal::Script>&>(script_);
    script = factory::BitcoinScript(chain_, std::move(elements), false, false);
    size_ = std::nullopt;

    return bool(script);
}

auto Input::AssociatedLocalNyms(
    const api::client::Blockchain& blockchain,
    std::vector<OTNymID>& output) const noexcept -> void
{
    std::for_each(std::begin(keys_), std::end(keys_), [&](const auto& key) {
        output.emplace_back(blockchain.Owner(key));
    });
}

auto Input::AssociatedRemoteContacts(
    const api::client::Blockchain& blockchain,
    std::vector<OTIdentifier>& output) const noexcept -> void
{
    const auto hashes = script_->LikelyPubkeyHashes(api_);
    std::for_each(std::begin(hashes), std::end(hashes), [&](const auto& hash) {
        auto contacts = blockchain.LookupContacts(hash);
        std::move(
            std::begin(contacts),
            std::end(contacts),
            std::back_inserter(output));
    });
}

auto Input::AssociatePreviousOutput(
    const api::client::Blockchain& blockchain,
    const proto::BlockchainTransactionOutput& in) noexcept -> bool
{
    if (previous_.Index() != in.index()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong previous output").Flush();

        return false;
    }

    if (false == bool(previous_output_)) {
        previous_output_ =
            factory::BitcoinTransactionOutput(api_, blockchain, chain_, in);
    }

    // NOTE this should only happen during unit testing
    if (keys_.empty()) {
        OT_ASSERT(0 < in.key_size());

        for (const auto& key : in.key()) {
            keys_.emplace(
                key.subaccount(),
                static_cast<api::client::blockchain::Subchain>(
                    static_cast<std::uint8_t>(key.subchain())),
                key.index());
        }
    }

    return bool(previous_output_);
}

auto Input::CalculateSize(const bool normalized) const noexcept -> std::size_t
{
    auto& output = normalized ? normalized_size_ : size_;

    if (false == output.has_value()) {
        const auto data = (0 < coinbase_.size()) ? coinbase_.size()
                                                 : script_->CalculateSize();
        const auto cs = blockchain::bitcoin::CompactSize(data);
        output = sizeof(previous_) + (normalized ? 1 : cs.Total()) +
                 sizeof(sequence_);
    }

    return output.value();
}

auto Input::ExtractElements(const filter::Type style) const noexcept
    -> std::vector<Space>
{
    auto output = std::vector<Space>{};

    if (Script::Position::Coinbase == script_->Role()) { return output; }

    switch (style) {
        case filter::Type::Extended_opentxs: {

            LogTrace(OT_METHOD)(__FUNCTION__)(": processing input script")
                .Flush();
            output = script_->ExtractElements(style);

            for (const auto& data : witness_) {
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

            [[fallthrough]];
        }
        case filter::Type::Basic_BCHVariant: {
            LogTrace(OT_METHOD)(__FUNCTION__)(": processing consumed outpoint")
                .Flush();
            auto it = reinterpret_cast<const std::byte*>(&previous_);
            output.emplace_back(it, it + sizeof(previous_));
        } break;
        case filter::Type::Basic_BIP158:
        default: {
            LogTrace(OT_METHOD)(__FUNCTION__)(": skipping input").Flush();
        }
    }

    LogTrace(OT_METHOD)(__FUNCTION__)(": extracted ")(output.size())(
        " elements")
        .Flush();

    return output;
}

auto Input::FindMatches(
    const ReadView txid,
    const FilterType type,
    const Patterns& txos,
    const Patterns& patterns) const noexcept -> Matches
{
    auto output = SetIntersection(api_, txid, patterns, ExtractElements(type));

    if (0 < txos.size()) {
        auto outpoint = std::vector<Space>{};
        auto it = reinterpret_cast<const std::byte*>(&previous_);
        outpoint.emplace_back(it, it + sizeof(previous_));
        auto temp = SetIntersection(api_, txid, txos, outpoint);
        output.insert(
            output.end(),
            std::make_move_iterator(temp.begin()),
            std::make_move_iterator(temp.end()));
    }

    std::for_each(
        std::begin(output), std::end(output), [this](const auto& match) {
            const auto& [txid, element] = match;
            const auto& [index, subchainID] = element;
            const auto& [subchain, account] = subchainID;
            keys_.emplace(KeyID{account->str(), subchain, index});
        });

    return output;
}

auto Input::GetPatterns() const noexcept -> std::vector<PatternID>
{
    return {std::begin(pubkey_hashes_), std::end(pubkey_hashes_)};
}

auto Input::index_elements(const api::client::Blockchain& blockchain) noexcept
    -> void
{
    auto& hashes =
        const_cast<boost::container::flat_set<PatternID>&>(pubkey_hashes_);
    const auto patterns = script_->ExtractPatterns(api_, blockchain);
    LogTrace(OT_METHOD)(__FUNCTION__)(": ")(patterns.size())(
        " pubkey hashes found:")
        .Flush();
    std::for_each(
        std::begin(patterns), std::end(patterns), [&](const auto& id) -> auto {
            hashes.emplace(id);
            LogTrace("    * ")(id).Flush();
        });
    const auto script = script_->RedeemScript();

    if (script) {
        auto scriptHash = Space{};
        script->CalculateHash160(api_, writer(scriptHash));
        const_cast<std::optional<PatternID>&>(script_hash_) =
            blockchain.IndexItem(reader(scriptHash));
    }
}

auto Input::Keys() const noexcept -> std::vector<KeyID>
{
    auto output = std::vector<KeyID>{};
    std::transform(
        std::begin(keys_), std::end(keys_), std::back_inserter(output), [
        ](const auto& key) -> auto { return key; });

    return output;
}

auto Input::MergeMetadata(
    const api::client::Blockchain& blockchain,
    const SerializeType& rhs) noexcept -> void
{
    std::for_each(
        std::begin(rhs.key()), std::end(rhs.key()), [this](const auto& key) {
            keys_.emplace(
                key.subaccount(),
                static_cast<api::client::blockchain::Subchain>(
                    static_cast<std::uint8_t>(key.subchain())),
                key.index());
        });

    if (rhs.has_spends() && (false == bool(previous_output_))) {
        previous_output_ = factory::BitcoinTransactionOutput(
            api_, blockchain, chain_, rhs.spends());
    }
}

auto Input::NetBalanceChange(
    const api::client::Blockchain& blockchain,
    const identifier::Nym& nym) const noexcept -> opentxs::Amount
{
    if (false == bool(previous_output_)) { return 0; }

    for (const auto& key : keys_) {
        if (blockchain.Owner(key) == nym) {
            return -1 * previous_output_->Value();
        }
    }

    return 0;
}

auto Input::ReplaceScript() noexcept -> bool
{
    if (false == bool(previous_output_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": previous output missing").Flush();

        return false;
    }

    const auto& output = *previous_output_;
    auto subscript = output.SigningSubscript();

    if (false == bool(subscript)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain signing subscript")
            .Flush();

        return false;
    }

    auto& script =
        const_cast<std::unique_ptr<const internal::Script>&>(script_);
    script.reset(subscript.release());
    size_ = std::nullopt;

    return true;
}

auto Input::serialize(const AllocateOutput destination, const bool normalized)
    const noexcept -> std::optional<std::size_t>
{
    if (!destination) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return std::nullopt;
    }

    const auto size = CalculateSize(normalized);
    auto output = destination(size);

    if (false == output.valid(size)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate output bytes")
            .Flush();

        return std::nullopt;
    }

    auto it = static_cast<std::byte*>(output.data());
    std::memcpy(static_cast<void*>(it), &previous_, sizeof(previous_));
    std::advance(it, sizeof(previous_));
    const auto isCoinbase{0 < coinbase_.size()};
    const auto cs = normalized ? blockchain::bitcoin::CompactSize(0)
                               : blockchain::bitcoin::CompactSize(
                                     isCoinbase ? coinbase_.size()
                                                : script_->CalculateSize());
    const auto csData = cs.Encode();
    std::memcpy(static_cast<void*>(it), csData.data(), csData.size());
    std::advance(it, csData.size());

    if (false == normalized) {
        if (isCoinbase) {
            std::memcpy(it, coinbase_.data(), coinbase_.size());
        } else {
            if (false == script_->Serialize(preallocated(cs.Value(), it))) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to serialize script")
                    .Flush();

                return std::nullopt;
            }
        }

        std::advance(it, cs.Value());
    }

    auto buf = be::little_uint32_buf_t{sequence_};
    std::memcpy(static_cast<void*>(it), &buf, sizeof(buf));

    return size;
}

auto Input::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, false);
}

auto Input::Serialize(
    const api::client::Blockchain& blockchain,
    const std::uint32_t index,
    SerializeType& out) const noexcept -> bool
{
    out.set_version(std::max(default_version_, serialize_version_));
    out.set_index(index);

    if (false == script_->Serialize(writer(*out.mutable_script()))) {

        return false;
    }

    out.set_sequence(sequence_);
    auto& outpoint = *out.mutable_previous();
    outpoint.set_version(outpoint_version_);
    outpoint.set_txid(std::string{previous_.Txid()});
    outpoint.set_index(previous_.Index());

    for (const auto& key : keys_) {
        const auto& [accountID, subchain, index] = key;
        auto& serializedKey = *out.add_key();
        serializedKey.set_version(key_version_);
        serializedKey.set_chain(Translate(chain_));
        serializedKey.set_nym(blockchain.Owner(key).str());
        serializedKey.set_subaccount(accountID);
        serializedKey.set_subchain(static_cast<std::uint32_t>(subchain));
        serializedKey.set_index(index);
    }

    for (const auto& id : pubkey_hashes_) { out.add_pubkey_hash(id); }

    if (script_hash_.has_value()) { out.set_script_hash(script_hash_.value()); }

    out.set_indexed(true);

    if (previous_output_) {
        auto& spends = *out.mutable_spends();
        previous_output_->Serialize(blockchain, spends);
    }

    return true;
}

auto Input::SerializeNormalized(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, true);
}

auto Input::SignatureVersion() const noexcept
    -> std::unique_ptr<internal::Input>
{
    return std::make_unique<Input>(
        *this, factory::BitcoinScript(chain_, ScriptElements{}, false, false));
}

auto Input::SignatureVersion(std::unique_ptr<internal::Script> subscript)
    const noexcept -> std::unique_ptr<internal::Input>
{
    return std::make_unique<Input>(*this, std::move(subscript));
}

auto Input::Spends() const noexcept(false) -> const internal::Output&
{
    if (previous_output_) { return *previous_output_; }

    throw std::out_of_range("No previous output");
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation

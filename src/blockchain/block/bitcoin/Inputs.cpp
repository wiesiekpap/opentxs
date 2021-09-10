// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/block/bitcoin/Inputs.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/iterator/Bidirectional.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/protobuf/BlockchainTransaction.pb.h"
#include "util/Container.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::block::bitcoin::implementation::Inputs::"

namespace opentxs::factory
{
using ReturnType = blockchain::block::bitcoin::implementation::Inputs;

auto BitcoinTransactionInputs(
    std::vector<std::unique_ptr<blockchain::block::bitcoin::internal::Input>>&&
        inputs,
    std::optional<std::size_t> size) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Inputs>
{
    try {

        return std::make_unique<ReturnType>(std::move(inputs), size);
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
Inputs::Inputs(InputList&& inputs, std::optional<std::size_t> size) noexcept(
    false)
    : inputs_(std::move(inputs))
    , cache_()
{
    for (const auto& input : inputs_) {
        if (false == bool(input)) { throw std::runtime_error("invalid input"); }
    }
}

Inputs::Inputs(const Inputs& rhs) noexcept
    : inputs_(clone(rhs.inputs_))
    , cache_(rhs.cache_)
{
}

auto Inputs::AnyoneCanPay(const std::size_t index) noexcept -> bool
{
    auto& inputs = const_cast<InputList&>(inputs_);

    try {
        auto replace = InputList{};
        replace.emplace_back(inputs.at(index).release());
        inputs.swap(replace);
        cache_.reset_size();

        return true;
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Invalid index").Flush();

        return false;
    }
}

auto Inputs::AssociatedLocalNyms(
    const api::client::Blockchain& blockchain,
    std::vector<OTNymID>& output) const noexcept -> void
{
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& item) {
            item->AssociatedLocalNyms(blockchain, output);
        });
}

auto Inputs::AssociatedRemoteContacts(
    const api::client::Blockchain& blockchain,
    std::vector<OTIdentifier>& output) const noexcept -> void
{
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& item) {
            item->AssociatedRemoteContacts(blockchain, output);
        });
}

auto Inputs::AssociatePreviousOutput(
    const api::client::Blockchain& blockchain,
    const std::size_t index,
    const proto::BlockchainTransactionOutput& output) noexcept -> bool
{
    try {

        return inputs_.at(index)->AssociatePreviousOutput(blockchain, output);
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Invalid index").Flush();

        return false;
    }
}

auto Inputs::CalculateSize(const bool normalized) const noexcept -> std::size_t
{
    return cache_.size(normalized, [&] {
        const auto cs = blockchain::bitcoin::CompactSize(size());

        return std::accumulate(
            cbegin(),
            cend(),
            cs.Size(),
            [=](const std::size_t& lhs, const auto& rhs) -> std::size_t {
                return lhs + rhs.CalculateSize(normalized);
            });
    });
}

auto Inputs::clone(const InputList& rhs) noexcept -> InputList
{
    auto output = InputList{};
    std::transform(
        std::begin(rhs),
        std::end(rhs),
        std::back_inserter(output),
        [](const auto& in) { return in->clone(); });

    return output;
}

auto Inputs::ExtractElements(const filter::Type style) const noexcept
    -> std::vector<Space>
{
    auto output = std::vector<Space>{};
    LogTrace(OT_METHOD)(__func__)(": processing ")(size())(" inputs").Flush();

    for (const auto& txin : *this) {
        auto temp = txin.ExtractElements(style);
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

auto Inputs::FindMatches(
    const ReadView txid,
    const FilterType type,
    const Patterns& txos,
    const ParsedPatterns& patterns) const noexcept -> Matches
{
    auto output = Matches{};
    auto& [inputs, outputs] = output;
    auto index{-1};

    for (const auto& txin : *this) {
        auto temp = txin.FindMatches(txid, type, txos, patterns);
        LogTrace(OT_METHOD)(__func__)(": Verified ")(
            temp.second.size() +
            temp.first.size())(" matches in input ")(++index)
            .Flush();
        inputs.insert(
            inputs.end(),
            std::make_move_iterator(temp.first.begin()),
            std::make_move_iterator(temp.first.end()));
        outputs.insert(
            outputs.end(),
            std::make_move_iterator(temp.second.begin()),
            std::make_move_iterator(temp.second.end()));
    }

    return output;
}

auto Inputs::GetPatterns() const noexcept -> std::vector<PatternID>
{
    auto output = std::vector<PatternID>{};
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& txin) {
            const auto patterns = txin->GetPatterns();
            output.insert(output.end(), patterns.begin(), patterns.end());
        });

    dedup(output);

    return output;
}

auto Inputs::Keys() const noexcept -> std::vector<KeyID>
{
    auto out = std::vector<KeyID>{};

    for (const auto& input : *this) {
        auto keys = input.Keys();
        std::move(keys.begin(), keys.end(), std::back_inserter(out));
        dedup(out);
    }

    return out;
}

auto Inputs::NetBalanceChange(
    const api::client::Blockchain& blockchain,
    const identifier::Nym& nym) const noexcept -> opentxs::Amount
{
    return std::accumulate(
        std::begin(inputs_),
        std::end(inputs_),
        opentxs::Amount{0},
        [&](const auto prev, const auto& input) -> auto {
            return prev + input->NetBalanceChange(blockchain, nym);
        });
}

auto Inputs::ReplaceScript(const std::size_t index) noexcept -> bool
{
    try {
        cache_.reset_size();

        return inputs_.at(index)->ReplaceScript();
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Invalid index").Flush();

        return false;
    }
}

auto Inputs::serialize(const AllocateOutput destination, const bool normalize)
    const noexcept -> std::optional<std::size_t>
{
    if (!destination) {
        LogOutput(OT_METHOD)(__func__)(": Invalid output allocator").Flush();

        return std::nullopt;
    }

    const auto size = CalculateSize(normalize);
    auto output = destination(size);

    if (false == output.valid(size)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to allocate output bytes")
            .Flush();

        return std::nullopt;
    }

    auto remaining{output.size()};
    const auto cs = blockchain::bitcoin::CompactSize(this->size()).Encode();
    auto it = static_cast<std::byte*>(output.data());
    std::memcpy(static_cast<void*>(it), cs.data(), cs.size());
    std::advance(it, cs.size());
    remaining -= cs.size();

    for (const auto& row : inputs_) {
        OT_ASSERT(row);

        const auto bytes =
            normalize ? row->SerializeNormalized(preallocated(remaining, it))
                      : row->Serialize(preallocated(remaining, it));

        if (false == bytes.has_value()) {
            LogOutput(OT_METHOD)(__func__)(": Failed to serialize input")
                .Flush();

            return std::nullopt;
        }

        std::advance(it, bytes.value());
        remaining -= bytes.value();
    }

    return size;
}

auto Inputs::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, false);
}

auto Inputs::Serialize(
    const api::client::Blockchain& blockchain,
    proto::BlockchainTransaction& destination) const noexcept -> bool
{
    auto index = std::uint32_t{0};

    for (const auto& input : inputs_) {
        OT_ASSERT(input);

        auto& out = *destination.add_input();

        if (false == input->Serialize(blockchain, index, out)) { return false; }

        ++index;
    }

    return true;
}

auto Inputs::SerializeNormalized(const AllocateOutput destination)
    const noexcept -> std::optional<std::size_t>
{
    return serialize(destination, true);
}

auto Inputs::SetKeyData(const KeyData& data) noexcept -> void
{
    for (auto& input : inputs_) { input->SetKeyData(data); }
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation

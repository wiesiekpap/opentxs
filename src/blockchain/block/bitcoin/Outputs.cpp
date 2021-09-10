// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "blockchain/block/bitcoin/Outputs.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/iterator/Bidirectional.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/protobuf/BlockchainTransaction.pb.h"
#include "util/Container.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::block::bitcoin::implementation::Outputs::"

namespace opentxs::factory
{
using ReturnType = blockchain::block::bitcoin::implementation::Outputs;

auto BitcoinTransactionOutputs(
    std::vector<std::unique_ptr<blockchain::block::bitcoin::internal::Output>>&&
        outputs,
    std::optional<std::size_t> size) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Outputs>
{
    try {

        return std::make_unique<ReturnType>(std::move(outputs), size);
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
Outputs::Outputs(
    OutputList&& outputs,
    std::optional<std::size_t> size) noexcept(false)
    : outputs_(std::move(outputs))
    , cache_()
{
    for (const auto& output : outputs_) {
        if (false == bool(output)) {
            throw std::runtime_error("invalid output");
        }
    }
}

Outputs::Outputs(const Outputs& rhs) noexcept
    : outputs_(clone(rhs.outputs_))
    , cache_(rhs.cache_)
{
}

auto Outputs::AssociatedLocalNyms(
    const api::client::Blockchain& blockchain,
    std::vector<OTNymID>& output) const noexcept -> void
{
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& item) {
            item->AssociatedLocalNyms(blockchain, output);
        });
}

auto Outputs::AssociatedRemoteContacts(
    const api::client::Blockchain& blockchain,
    std::vector<OTIdentifier>& output) const noexcept -> void
{
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& item) {
            item->AssociatedRemoteContacts(blockchain, output);
        });
}

auto Outputs::CalculateSize() const noexcept -> std::size_t
{
    return cache_.size([&] {
        const auto cs = blockchain::bitcoin::CompactSize(size());

        return std::accumulate(
            cbegin(),
            cend(),
            cs.Size(),
            [](const std::size_t& lhs, const auto& rhs) -> std::size_t {
                return lhs + rhs.CalculateSize();
            });
    });
}

auto Outputs::clone(const OutputList& rhs) noexcept -> OutputList
{
    auto output = OutputList{};
    std::transform(
        std::begin(rhs),
        std::end(rhs),
        std::back_inserter(output),
        [](const auto& in) { return in->clone(); });

    return output;
}

auto Outputs::ExtractElements(const filter::Type style) const noexcept
    -> std::vector<Space>
{
    auto output = std::vector<Space>{};
    LogTrace(OT_METHOD)(__func__)(": processing ")(size())(" outputs").Flush();

    for (const auto& txout : *this) {
        auto temp = txout.ExtractElements(style);
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

auto Outputs::FindMatches(
    const ReadView txid,
    const FilterType type,
    const ParsedPatterns& patterns) const noexcept -> Matches
{
    auto output = Matches{};
    auto index{-1};

    for (const auto& txout : *this) {
        auto temp = txout.FindMatches(txid, type, patterns);
        LogTrace(OT_METHOD)(__func__)(": Verified ")(temp.second.size())(
            " matches in output ")(++index)
            .Flush();
        output.second.insert(
            output.second.end(),
            std::make_move_iterator(temp.second.begin()),
            std::make_move_iterator(temp.second.end()));
    }

    return output;
}

auto Outputs::ForTestingOnlyAddKey(
    const std::size_t index,
    const blockchain::crypto::Key& key) noexcept -> bool
{
    try {
        outputs_.at(index)->ForTestingOnlyAddKey(key);

        return true;
    } catch (...) {

        return false;
    }
}

auto Outputs::GetPatterns() const noexcept -> std::vector<PatternID>
{
    auto output = std::vector<PatternID>{};
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& txout) {
            const auto patterns = txout->GetPatterns();
            output.insert(output.end(), patterns.begin(), patterns.end());
        });

    dedup(output);

    return output;
}

auto Outputs::Keys() const noexcept -> std::vector<KeyID>
{
    auto out = std::vector<KeyID>{};

    for (const auto& output : *this) {
        auto keys = output.Keys();
        std::move(keys.begin(), keys.end(), std::back_inserter(out));
        dedup(out);
    }

    return out;
}

auto Outputs::NetBalanceChange(
    const api::client::Blockchain& blockchain,
    const identifier::Nym& nym) const noexcept -> opentxs::Amount
{
    return std::accumulate(
        std::begin(outputs_),
        std::end(outputs_),
        opentxs::Amount{0},
        [&](const auto prev, const auto& output) -> auto {
            return prev + output->NetBalanceChange(blockchain, nym);
        });
}

auto Outputs::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    if (!destination) {
        LogOutput(OT_METHOD)(__func__)(": Invalid output allocator").Flush();

        return std::nullopt;
    }

    const auto size = CalculateSize();
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

    for (const auto& row : outputs_) {
        OT_ASSERT(row);

        const auto bytes = row->Serialize(preallocated(remaining, it));

        if (false == bytes.has_value()) {
            LogOutput(OT_METHOD)(__func__)(": Failed to serialize script")
                .Flush();

            return std::nullopt;
        }

        std::advance(it, bytes.value());
        remaining -= bytes.value();
    }

    return size;
}

auto Outputs::Serialize(
    const api::client::Blockchain& blockchain,
    proto::BlockchainTransaction& destination) const noexcept -> bool
{
    for (const auto& output : outputs_) {
        OT_ASSERT(output);

        auto& out = *destination.add_output();

        if (false == output->Serialize(blockchain, out)) { return false; }
    }

    return true;
}

auto Outputs::SetKeyData(const KeyData& data) noexcept -> void
{
    for (auto& output : outputs_) { output->SetKeyData(data); }
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation

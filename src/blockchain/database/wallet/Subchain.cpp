// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "blockchain/database/wallet/Subchain.hpp"  // IWYU pragma: associated

#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <iterator>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

// #define OT_METHOD "opentxs::blockchain::database::SubchainData::"

namespace opentxs::blockchain::database::wallet
{
struct SubchainData::Imp {
    auto GetSubchainID(const NodeID& balanceNode, const Subchain subchain)
        const noexcept -> pSubchainID
    {
        return subchain_id(balanceNode, subchain);
    }
    auto GetSubchainIndex(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> pSubchainID
    {
        auto lock = Lock{lock_};
        auto output = subchain_index(lock, balanceNode, subchain, type);
        reverse_index_.try_emplace(output, balanceNode, subchain, type);

        return output;
    }
    auto GetMutex() const noexcept -> std::mutex& { return lock_; }
    auto GetPatterns(const SubchainIndex& subchain) const noexcept -> Patterns
    {
        auto lock = Lock{lock_};

        try {
            const auto& patterns = get_patterns(lock, subchain);

            return load_patterns(lock, subchain, patterns);
        } catch (...) {

            return {};
        }
    }
    auto GetUntestedPatterns(
        const SubchainIndex& subchain,
        const ReadView blockID) const noexcept -> Patterns
    {
        auto lock = Lock{lock_};

        try {
            const auto& allPatterns = get_patterns(lock, subchain);
            auto effectiveIDs = std::vector<pPatternID>{};

            try {
                const auto& matchedPatterns =
                    match_index_.at(api_.Factory().Data(blockID));
                std::set_difference(
                    std::begin(allPatterns),
                    std::end(allPatterns),
                    std::begin(matchedPatterns),
                    std::end(matchedPatterns),
                    std::back_inserter(effectiveIDs));
            } catch (...) {

                return load_patterns(lock, subchain, allPatterns);
            }

            return load_patterns(lock, subchain, effectiveIDs);
        } catch (...) {

            return {};
        }
    }
    auto Reorg(
        const Lock& lock,
        const SubchainIndex& subchain,
        const block::Height lastGoodHeight) const noexcept(false) -> bool
    {
        try {
            auto& scanned = last_scanned_.at(subchain);

            if (const auto& height = scanned.first; height < lastGoodHeight) {
                // noop
            } else if (height > lastGoodHeight) {
                scanned.first = lastGoodHeight;
            } else {
                scanned.first = std::min<block::Height>(lastGoodHeight - 1, 0);
            }

            // FIXME use header oracle to set correct block hash if
            // scanned.first is modified
        } catch (...) {
        }

        return false;
    }
    auto SetDefaultFilterType(const FilterType type) const noexcept -> bool
    {
        auto lock = Lock{lock_};
        const_cast<FilterType&>(default_filter_type_) = type;

        return true;
    }
    auto SubchainAddElements(
        const SubchainIndex& subchain,
        const ElementMap& elements) const noexcept -> bool
    {
        auto lock = Lock{lock_};
        auto newIndices = std::vector<OTIdentifier>{};
        auto highest = Bip32Index{};

        for (const auto& [index, patterns] : elements) {
            auto patternID = pattern_id(subchain, index);
            auto& vector = patterns_[patternID];
            newIndices.emplace_back(std::move(patternID));
            highest = std::max(highest, index);

            for (const auto& pattern : patterns) {
                vector.emplace_back(index, pattern);
            }
        }

        last_indexed_[subchain] = highest;
        auto& index = subchain_pattern_index_[subchain];

        for (auto& id : newIndices) { index.emplace(std::move(id)); }

        return true;
    }
    auto SubchainLastIndexed(const SubchainIndex& subchain) const noexcept
        -> std::optional<Bip32Index>
    {
        auto lock = Lock{lock_};

        try {
            return last_indexed_.at(subchain);
        } catch (...) {
            return {};
        }
    }
    auto SubchainLastScanned(const SubchainIndex& subchain) const noexcept
        -> block::Position
    {
        auto lock = Lock{lock_};

        try {
            return last_scanned_.at(subchain);
        } catch (...) {
            return make_blank<block::Position>::value(api_);
        }
    }
    auto SubchainMatchBlock(
        const SubchainIndex& subchain,
        const MatchingIndices& indices,
        const ReadView blockID) const noexcept -> bool
    {
        auto lock = Lock{lock_};
        auto& matchSet = match_index_[api_.Factory().Data(blockID)];

        for (const auto& index : indices) {
            matchSet.emplace(pattern_id(subchain, index));
        }

        return true;
    }
    auto SubchainSetLastScanned(
        const SubchainIndex& subchain,
        const block::Position& position) const noexcept -> bool
    {
        auto lock = Lock{lock_};
        auto& map = last_scanned_;
        auto it = map.find(subchain);

        if (map.end() == it) {
            map.emplace(subchain, position);

            return true;
        } else {
            it->second = position;

            return true;
        }
    }
    auto Type() const noexcept -> FilterType
    {
        auto lock = Lock{lock_};

        return default_filter_type_;
    }

    Imp(const api::Core& api) noexcept
        : api_(api)
        , default_filter_type_()
        , current_version_(1)
        , lock_()
        , last_indexed_()
        , last_scanned_()
        , subchain_version_()
        , patterns_()
        , subchain_pattern_index_()
        , match_index_()
        , reverse_index_()
    {
        // TODO persist default_filter_type_ and reindex various tables
        // if the type provided by the filter oracle has changed
    }

private:
    using pSubchainVersionIndex = OTIdentifier;
    using PatternID = Identifier;
    using pPatternID = OTIdentifier;
    using Pattern = Parent::Pattern;
    using SubchainIndexMap = std::map<pSubchainIndex, Bip32Index>;
    using PositionMap = std::map<pSubchainIndex, block::Position>;
    using VersionIndex = std::map<pSubchainVersionIndex, VersionNumber>;
    using PatternMap =
        std::map<pPatternID, std::vector<std::pair<Bip32Index, Space>>>;
    using IDSet = boost::container::flat_set<pPatternID>;
    using SubchainPatternIndex = std::map<pSubchainIndex, IDSet>;
    using MatchIndex = std::map<block::pHash, IDSet>;
    using ReverseIndex =
        std::map<pSubchainIndex, std::tuple<pNodeID, Subchain, FilterType>>;

    const api::Core& api_;
    const FilterType default_filter_type_;
    const VersionNumber current_version_;
    mutable std::mutex lock_;
    mutable SubchainIndexMap last_indexed_;
    mutable PositionMap last_scanned_;
    mutable VersionIndex subchain_version_;
    mutable PatternMap patterns_;
    mutable SubchainPatternIndex subchain_pattern_index_;
    mutable MatchIndex match_index_;
    mutable ReverseIndex reverse_index_;

    auto check_subchain_version(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> void
    {
        // TODO compared stored value of the subchain's version to the default
        // version and reindex if necessary.
    }

    auto get_patterns(const Lock& lock, const SubchainIndex& subchain) const
        noexcept(false) -> const IDSet&
    {
        return subchain_pattern_index_.at(subchain);
    }
    template <typename PatternList>
    auto load_patterns(
        const Lock& lock,
        const SubchainIndex& subchain,
        const PatternList& patterns) const noexcept -> Patterns
    {
        auto output = Patterns{};
        const auto [node, sub, type] = reverse_index_.at(subchain);

        for (const auto& patternID : patterns) {
            try {
                for (const auto& [index, pattern] : patterns_.at(patternID)) {
                    output.emplace_back(Pattern{{index, {sub, node}}, pattern});
                }
            } catch (...) {
            }
        }

        return output;
    }
    auto pattern_id(const SubchainIndex& subchain, const Bip32Index index)
        const noexcept -> pPatternID
    {
        auto preimage = OTData{subchain};
        preimage->Concatenate(&index, sizeof(index));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }
    auto subchain_id(const NodeID& nodeID, const Subchain subchain)
        const noexcept -> pSubchainID
    {
        auto preimage = OTData{nodeID};
        preimage->Concatenate(&subchain, sizeof(subchain));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }
    auto subchain_index(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> pSubchainIndex
    {
        auto preimage = OTData{balanceNode};
        preimage->Concatenate(&subchain, sizeof(subchain));
        preimage->Concatenate(&type, sizeof(type));
        preimage->Concatenate(&version, sizeof(version));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }
    auto subchain_index(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> pSubchainIndex
    {
        check_subchain_version(lock, balanceNode, subchain, type);

        return subchain_index(balanceNode, subchain, type, current_version_);
    }
    auto subchain_version(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> VersionNumber
    {
        const auto id = subchain_version_index(balanceNode, subchain, type);

        try {

            return subchain_version_.at(id);
        } catch (...) {

            return 0;
        }
    }
    auto subchain_version_index(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> pSubchainVersionIndex
    {
        auto preimage = OTData{balanceNode};
        preimage->Concatenate(&subchain, sizeof(subchain));
        preimage->Concatenate(&type, sizeof(type));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }
};

SubchainData::SubchainData(const api::Core& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
    OT_ASSERT(imp_);
}

auto SubchainData::GetIndex(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> pSubchainIndex
{
    return imp_->GetSubchainIndex(balanceNode, subchain, type);
}

auto SubchainData::GetSubchainID(
    const NodeID& balanceNode,
    const Subchain subchain) const noexcept -> pSubchainID
{
    return imp_->GetSubchainID(balanceNode, subchain);
}

auto SubchainData::GetMutex() const noexcept -> std::mutex&
{
    return imp_->GetMutex();
}

auto SubchainData::GetPatterns(const SubchainIndex& subchain) const noexcept
    -> Patterns
{
    return imp_->GetPatterns(subchain);
}

auto SubchainData::GetUntestedPatterns(
    const SubchainIndex& subchain,
    const ReadView blockID) const noexcept -> Patterns
{
    return imp_->GetUntestedPatterns(subchain, blockID);
}

auto SubchainData::Reorg(
    const Lock& lock,
    const SubchainIndex& subchain,
    const block::Height lastGoodHeight) const noexcept(false) -> bool
{
    return imp_->Reorg(lock, subchain, lastGoodHeight);
}

auto SubchainData::SetDefaultFilterType(const FilterType type) const noexcept
    -> bool
{
    return imp_->SetDefaultFilterType(type);
}

auto SubchainData::SubchainAddElements(
    const SubchainIndex& subchain,
    const ElementMap& elements) const noexcept -> bool
{
    return imp_->SubchainAddElements(subchain, elements);
}

auto SubchainData::SubchainLastIndexed(
    const SubchainIndex& subchain) const noexcept -> std::optional<Bip32Index>
{
    return imp_->SubchainLastIndexed(subchain);
}

auto SubchainData::SubchainLastScanned(
    const SubchainIndex& subchain) const noexcept -> block::Position
{
    return imp_->SubchainLastScanned(subchain);
}

auto SubchainData::SubchainMatchBlock(
    const SubchainIndex& subchain,
    const MatchingIndices& indices,
    const ReadView blockID) const noexcept -> bool
{
    return imp_->SubchainMatchBlock(subchain, indices, blockID);
}

auto SubchainData::SubchainSetLastScanned(
    const SubchainIndex& subchain,
    const block::Position& position) const noexcept -> bool
{
    return imp_->SubchainSetLastScanned(subchain, position);
}

auto SubchainData::Type() const noexcept -> FilterType { return imp_->Type(); }

SubchainData::~SubchainData() = default;
}  // namespace opentxs::blockchain::database::wallet

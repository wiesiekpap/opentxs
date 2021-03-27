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
    auto GetID(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> pSubchainID
    {
        return subchain_id(balanceNode, subchain, type, version);
    }
    auto GetID(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> pSubchainID
    {
        auto lock = Lock{lock_};

        return subchain_id(
            balanceNode,
            subchain,
            default_filter_type_,
            subchain_index_version(
                lock, balanceNode, subchain, default_filter_type_));
    }
    auto GetID(const NodeID& balanceNode, const Subchain subchain)
        const noexcept -> pSubchainID
    {
        auto lock = Lock{lock_};

        return subchain_id(lock, balanceNode, subchain);
    }
    auto GetMutex() const noexcept -> std::mutex& { return lock_; }
    auto GetPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> Patterns
    {
        auto lock = Lock{lock_};

        try {
            const auto& patterns =
                get_patterns(lock, balanceNode, subchain, type, version);

            return load_patterns(lock, balanceNode, subchain, patterns);
        } catch (...) {

            return {};
        }
    }
    auto GetUntestedPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ReadView blockID,
        const VersionNumber version) const noexcept -> Patterns
    {
        auto lock = Lock{lock_};

        try {
            const auto& allPatterns =
                get_patterns(lock, balanceNode, subchain, type, version);
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

                return load_patterns(lock, balanceNode, subchain, allPatterns);
            }

            return load_patterns(lock, balanceNode, subchain, effectiveIDs);
        } catch (...) {

            return {};
        }
    }
    auto Reorg(
        const Lock& lock,
        const SubchainID& subchain,
        const block::Height lastGoodHeight) const noexcept(false) -> bool
    {
        auto& scanned = subchain_last_scanned_.at(subchain);
        auto& processed = subchain_last_processed_.at(subchain);

        if (const auto& height = scanned.first; height < lastGoodHeight) {
            // noop
        } else if (height > lastGoodHeight) {
            scanned.first = lastGoodHeight;
        } else {
            scanned.first = std::min<block::Height>(lastGoodHeight - 1, 0);
        }

        if (const auto& height = processed.first; height < lastGoodHeight) {
            return true;
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
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ElementMap& elements,
        const VersionNumber version) const noexcept -> bool
    {
        auto lock = Lock{lock_};
        subchain_version_[subchain_version_index(balanceNode, subchain, type)] =
            version;
        auto subchainID = subchain_id(balanceNode, subchain, type, version);
        auto newIndices = std::vector<OTIdentifier>{};
        auto highest = Bip32Index{};

        for (const auto& [index, patterns] : elements) {
            auto patternID = pattern_id(subchainID, index);
            auto& vector = patterns_[patternID];
            newIndices.emplace_back(std::move(patternID));
            highest = std::max(highest, index);

            for (const auto& pattern : patterns) {
                vector.emplace_back(index, pattern);
            }
        }

        subchain_last_indexed_[subchainID] = highest;
        auto& index = subchain_pattern_index_[subchainID];

        for (auto& id : newIndices) { index.emplace(std::move(id)); }

        return true;
    }
    auto SubchainDropIndex(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> bool
    {
        auto lock = Lock{lock_};
        const auto subchainID =
            subchain_id(balanceNode, subchain, type, version);

        try {
            for (const auto& patternID :
                 subchain_pattern_index_.at(subchainID)) {
                patterns_.erase(patternID);

                for (auto& [block, set] : match_index_) {
                    set.erase(patternID);
                }
            }
        } catch (...) {
        }

        subchain_pattern_index_.erase(subchainID);
        subchain_last_indexed_.erase(subchainID);
        subchain_version_.erase(
            subchain_version_index(balanceNode, subchain, type));

        return true;
    }
    auto SubchainIndexVersion(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> VersionNumber
    {
        auto lock = Lock{lock_};

        return subchain_index_version(lock, balanceNode, subchain, type);
    }
    auto SubchainLastIndexed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> std::optional<Bip32Index>
    {
        auto lock = Lock{lock_};
        const auto subchainID =
            subchain_id(balanceNode, subchain, type, version);

        try {
            return subchain_last_indexed_.at(subchainID);
        } catch (...) {
            return {};
        }
    }
    auto SubchainLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position
    {
        auto lock = Lock{lock_};

        try {
            return subchain_last_processed_.at(
                subchain_version_index(balanceNode, subchain, type));
        } catch (...) {
            return make_blank<block::Position>::value(api_);
        }
    }
    auto SubchainLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position
    {
        auto lock = Lock{lock_};

        try {
            return subchain_last_scanned_.at(
                subchain_version_index(balanceNode, subchain, type));
        } catch (...) {
            return make_blank<block::Position>::value(api_);
        }
    }
    auto SubchainMatchBlock(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const MatchingIndices& indices,
        const ReadView blockID,
        const VersionNumber version) const noexcept -> bool
    {
        auto lock = Lock{lock_};
        auto& matchSet = match_index_[api_.Factory().Data(blockID)];

        for (const auto& index : indices) {
            matchSet.emplace(pattern_id(
                subchain_id(balanceNode, subchain, type, version), index));
        }

        return true;
    }
    auto SubchainSetLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool
    {
        auto lock = Lock{lock_};
        auto& map = subchain_last_processed_;
        auto id = subchain_version_index(balanceNode, subchain, type);
        auto it = map.find(id);

        if (map.end() == it) {
            map.emplace(std::move(id), position);

            return true;
        } else {
            it->second = position;

            return true;
        }
    }
    auto SubchainSetLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool
    {
        auto lock = Lock{lock_};
        auto& map = subchain_last_scanned_;
        auto id = subchain_version_index(balanceNode, subchain, type);
        auto it = map.find(id);

        if (map.end() == it) {
            map.emplace(std::move(id), position);

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
        , lock_()
        , subchain_last_indexed_()
        , subchain_last_scanned_()
        , subchain_last_processed_()
        , subchain_version_()
        , patterns_()
        , subchain_pattern_index_()
        , match_index_()
    {
        // TODO persist default_filter_type_ and reindex various tables
        // if the type provided by the filter oracle has changed
    }

private:
    using pSubchainID = OTIdentifier;
    using PatternID = Identifier;
    using pPatternID = OTIdentifier;
    using Pattern = Parent::Pattern;
    using SubchainIndexMap = std::map<pSubchainID, VersionNumber>;
    using PositionMap = std::map<OTIdentifier, block::Position>;
    using VersionIndex = std::map<OTIdentifier, VersionNumber>;
    using PatternMap =
        std::map<pPatternID, std::vector<std::pair<Bip32Index, Space>>>;
    using IDSet = boost::container::flat_set<pPatternID>;
    using SubchainPatternIndex = std::map<pSubchainID, IDSet>;
    using MatchIndex = std::map<block::pHash, IDSet>;

    const api::Core& api_;
    const FilterType default_filter_type_;
    mutable std::mutex lock_;
    mutable SubchainIndexMap subchain_last_indexed_;
    mutable PositionMap subchain_last_scanned_;
    mutable PositionMap subchain_last_processed_;
    mutable VersionIndex subchain_version_;
    mutable PatternMap patterns_;
    mutable SubchainPatternIndex subchain_pattern_index_;
    mutable MatchIndex match_index_;

    auto get_patterns(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept(false) -> const IDSet&
    {
        return subchain_pattern_index_.at(
            subchain_id(balanceNode, subchain, type, version));
    }
    template <typename PatternList>
    auto load_patterns(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain,
        const PatternList& patterns) const noexcept -> Patterns
    {
        auto output = Patterns{};

        for (const auto& patternID : patterns) {
            try {
                for (const auto& [index, pattern] : patterns_.at(patternID)) {
                    output.emplace_back(
                        Pattern{{index, {subchain, balanceNode}}, pattern});
                }
            } catch (...) {
            }
        }

        return output;
    }
    auto pattern_id(const SubchainID& subchain, const Bip32Index index)
        const noexcept -> pPatternID
    {
        auto preimage = OTData{subchain};
        preimage->Concatenate(&index, sizeof(index));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }
    auto subchain_id(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> pSubchainID
    {
        auto preimage = OTData{balanceNode};
        preimage->Concatenate(&subchain, sizeof(subchain));
        preimage->Concatenate(&type, sizeof(type));
        preimage->Concatenate(&version, sizeof(version));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }
    auto subchain_id(
        const Lock& lock,
        const NodeID& nodeID,
        const Subchain subchain) const noexcept -> pSubchainID
    {
        return subchain_id(
            nodeID,
            subchain,
            default_filter_type_,
            subchain_index_version(
                lock, nodeID, subchain, default_filter_type_));
    }
    auto subchain_index_version(
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
        const FilterType type) const noexcept -> pSubchainID
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

auto SubchainData::GetID(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> pSubchainID
{
    return imp_->GetID(balanceNode, subchain, type, version);
}

auto SubchainData::GetID(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> pSubchainID
{
    return imp_->GetID(balanceNode, subchain, type);
}

auto SubchainData::GetID(const NodeID& balanceNode, const Subchain subchain)
    const noexcept -> pSubchainID
{
    return imp_->GetID(balanceNode, subchain);
}

auto SubchainData::GetMutex() const noexcept -> std::mutex&
{
    return imp_->GetMutex();
}

auto SubchainData::GetPatterns(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> Patterns
{
    return imp_->GetPatterns(balanceNode, subchain, type, version);
}

auto SubchainData::GetUntestedPatterns(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const ReadView blockID,
    const VersionNumber version) const noexcept -> Patterns
{
    return imp_->GetUntestedPatterns(
        balanceNode, subchain, type, blockID, version);
}

auto SubchainData::Reorg(
    const Lock& lock,
    const SubchainID& subchain,
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
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const ElementMap& elements,
    const VersionNumber version) const noexcept -> bool
{
    return imp_->SubchainAddElements(
        balanceNode, subchain, type, elements, version);
}

auto SubchainData::SubchainDropIndex(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> bool
{
    return imp_->SubchainDropIndex(balanceNode, subchain, type, version);
}

auto SubchainData::SubchainIndexVersion(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> VersionNumber
{
    return imp_->SubchainIndexVersion(balanceNode, subchain, type);
}

auto SubchainData::SubchainLastIndexed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> std::optional<Bip32Index>
{
    return imp_->SubchainLastIndexed(balanceNode, subchain, type, version);
}

auto SubchainData::SubchainLastProcessed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> block::Position
{
    return imp_->SubchainLastProcessed(balanceNode, subchain, type);
}

auto SubchainData::SubchainLastScanned(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> block::Position
{
    return imp_->SubchainLastScanned(balanceNode, subchain, type);
}

auto SubchainData::SubchainMatchBlock(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const MatchingIndices& indices,
    const ReadView blockID,
    const VersionNumber version) const noexcept -> bool
{
    return imp_->SubchainMatchBlock(
        balanceNode, subchain, type, indices, blockID, version);
}

auto SubchainData::SubchainSetLastProcessed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const block::Position& position) const noexcept -> bool
{
    return imp_->SubchainSetLastProcessed(
        balanceNode, subchain, type, position);
}

auto SubchainData::SubchainSetLastScanned(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const block::Position& position) const noexcept -> bool
{
    return imp_->SubchainSetLastScanned(balanceNode, subchain, type, position);
}

auto SubchainData::Type() const noexcept -> FilterType { return imp_->Type(); }

SubchainData::~SubchainData() = default;
}  // namespace opentxs::blockchain::database::wallet

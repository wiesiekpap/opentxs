// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CLIENT_FILTERORACLE_HPP
#define OPENTXS_BLOCKCHAIN_CLIENT_FILTERORACLE_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace proto
{
class GCS;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace client
{
struct GCS {
    using Targets = std::vector<ReadView>;
    using Matches = std::vector<Targets::const_iterator>;

    /// Serialized filter only, no element count
    OPENTXS_EXPORT virtual Space Compressed() const noexcept = 0;
    OPENTXS_EXPORT virtual std::uint32_t ElementCount() const noexcept = 0;
    /// Element count as CompactSize followed by serialized filter
    OPENTXS_EXPORT virtual OTData Encode() const noexcept = 0;
    OPENTXS_EXPORT virtual filter::pHash Hash() const noexcept = 0;
    OPENTXS_EXPORT virtual filter::pHeader Header(
        const ReadView previous) const noexcept = 0;
    OPENTXS_EXPORT virtual Matches Match(const Targets&) const noexcept = 0;
    virtual proto::GCS Serialize() const noexcept = 0;
    OPENTXS_EXPORT virtual bool Test(const Data& target) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Test(const ReadView target) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Test(
        const std::vector<OTData>& targets) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Test(
        const std::vector<Space>& targets) const noexcept = 0;

    virtual ~GCS() = default;
};

class FilterOracle
{
public:
    OPENTXS_EXPORT virtual filter::Type DefaultType() const noexcept = 0;
    OPENTXS_EXPORT virtual block::Position FilterTip(
        const filter::Type type) const noexcept = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<const GCS> LoadFilter(
        const filter::Type type,
        const block::Hash& block) const noexcept = 0;
    OPENTXS_EXPORT virtual filter::pHeader LoadFilterHeader(
        const filter::Type type,
        const block::Hash& block) const noexcept = 0;

    virtual ~FilterOracle() = default;

protected:
    FilterOracle() noexcept = default;

private:
    FilterOracle(const FilterOracle&) = delete;
    FilterOracle(FilterOracle&&) = delete;
    FilterOracle& operator=(const FilterOracle&) = delete;
    FilterOracle& operator=(FilterOracle&&) = delete;
};
}  // namespace client
}  // namespace blockchain
}  // namespace opentxs
#endif

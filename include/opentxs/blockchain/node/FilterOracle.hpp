// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CLIENT_FILTERORACLE_HPP
#define OPENTXS_BLOCKCHAIN_CLIENT_FILTERORACLE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

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
namespace node
{
struct OPENTXS_EXPORT GCS {
    using Targets = std::vector<ReadView>;
    using Matches = std::vector<Targets::const_iterator>;

    /// Serialized filter only, no element count
    virtual auto Compressed() const noexcept -> Space = 0;
    virtual auto ElementCount() const noexcept -> std::uint32_t = 0;
    /// Element count as CompactSize followed by serialized filter
    virtual auto Encode() const noexcept -> OTData = 0;
    virtual auto Hash() const noexcept -> filter::pHash = 0;
    virtual auto Header(const ReadView previous) const noexcept
        -> filter::pHeader = 0;
    virtual auto Match(const Targets&) const noexcept -> Matches = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(proto::GCS& out) const noexcept
        -> bool = 0;
    virtual auto Serialize(AllocateOutput out) const noexcept -> bool = 0;
    virtual auto Test(const Data& target) const noexcept -> bool = 0;
    virtual auto Test(const ReadView target) const noexcept -> bool = 0;
    virtual auto Test(const std::vector<OTData>& targets) const noexcept
        -> bool = 0;
    virtual auto Test(const std::vector<Space>& targets) const noexcept
        -> bool = 0;

    virtual ~GCS() = default;
};

class OPENTXS_EXPORT FilterOracle
{
public:
    virtual auto DefaultType() const noexcept -> filter::Type = 0;
    virtual auto FilterTip(const filter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto LoadFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> std::unique_ptr<const GCS> = 0;
    virtual auto LoadFilterHeader(
        const filter::Type type,
        const block::Hash& block) const noexcept -> filter::pHeader = 0;

    virtual ~FilterOracle() = default;

protected:
    FilterOracle() noexcept = default;

private:
    FilterOracle(const FilterOracle&) = delete;
    FilterOracle(FilterOracle&&) = delete;
    auto operator=(const FilterOracle&) -> FilterOracle& = delete;
    auto operator=(FilterOracle&&) -> FilterOracle& = delete;
};
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
#endif

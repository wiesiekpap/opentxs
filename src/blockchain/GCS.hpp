// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/protobuf/GCS.pb.h"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace proto
{
class GCS;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::blockchain::implementation
{
class GCS final : virtual public node::GCS
{
public:
    auto Compressed() const noexcept -> Space final;
    auto ElementCount() const noexcept -> std::uint32_t final { return count_; }
    auto Encode() const noexcept -> OTData final;
    auto Hash() const noexcept -> OTData final;
    auto Header(const ReadView previous) const noexcept -> OTData final;
    auto Match(const Targets&) const noexcept -> Matches final;
    auto Serialize(proto::GCS& out) const noexcept -> bool final;
    auto Serialize(AllocateOutput out) const noexcept -> bool final;
    auto Test(const Data& target) const noexcept -> bool final;
    auto Test(const ReadView target) const noexcept -> bool final;
    auto Test(const std::vector<OTData>& targets) const noexcept -> bool final;
    auto Test(const std::vector<Space>& targets) const noexcept -> bool final;

    GCS(const api::Core& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t filterElementCount,
        const ReadView key,
        const ReadView encoded)
    noexcept(false);
    GCS(const api::Core& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const ReadView key,
        const std::vector<ReadView>& elements)
    noexcept(false);

    ~GCS() final = default;

private:
    using Elements = std::vector<std::uint64_t>;

    const VersionNumber version_;
    const api::Core& api_;
    const std::uint8_t bits_;
    const std::uint32_t false_positive_rate_;
    const std::uint32_t count_;
    const std::optional<Elements> elements_;
    const OTData compressed_;
    const OTData key_;

    static auto transform(const std::vector<OTData>& in) noexcept
        -> std::vector<ReadView>;
    static auto transform(const std::vector<Space>& in) noexcept
        -> std::vector<ReadView>;

    auto decompress() const noexcept -> const Elements&;
    auto hashed_set_construct(const std::vector<OTData>& elements)
        const noexcept -> std::vector<std::uint64_t>;
    auto hashed_set_construct(const std::vector<Space>& elements) const noexcept
        -> std::vector<std::uint64_t>;
    auto hashed_set_construct(const std::vector<ReadView>& elements)
        const noexcept -> std::vector<std::uint64_t>;
    auto test(const std::vector<std::uint64_t>& targetHashes) const noexcept
        -> bool;
    auto hash_to_range(const ReadView in) const noexcept -> std::uint64_t;

    GCS() = delete;
    GCS(const GCS&) = delete;
    GCS(GCS&&) = delete;
    auto operator=(const GCS&) -> GCS& = delete;
    auto operator=(GCS&&) -> GCS& = delete;
};
}  // namespace opentxs::blockchain::implementation

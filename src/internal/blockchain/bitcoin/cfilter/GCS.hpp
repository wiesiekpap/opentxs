// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class GCS;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::internal
{
class GCS
{
public:
    virtual auto Serialize(proto::GCS& out) const noexcept -> bool = 0;

    virtual ~GCS() = default;
};
}  // namespace opentxs::blockchain::internal

namespace opentxs::gcs
{
using Elements = Vector<std::uint64_t>;

auto GolombDecode(
    const std::uint32_t N,
    const std::uint8_t P,
    const Vector<std::byte>& encoded,
    alloc::Default alloc) noexcept(false) -> Elements;
auto GolombEncode(
    const std::uint8_t P,
    const Elements& hashedSet,
    alloc::Default alloc) noexcept(false) -> Vector<std::byte>;
auto HashToRange(
    const api::Session& api,
    const ReadView key,
    const std::uint64_t range,
    const ReadView item) noexcept(false) -> std::uint64_t;
auto HashedSetConstruct(
    const api::Session& api,
    const ReadView key,
    const std::uint32_t N,
    const std::uint32_t M,
    const blockchain::GCS::Targets& items,
    alloc::Default alloc) noexcept(false) -> Elements;
}  // namespace opentxs::gcs

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/node/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
}  // namespace block

class GCS;
}  // namespace blockchain

namespace network
{
namespace p2p
{
class Data;
}  // namespace p2p
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class FilterOracle : virtual public node::FilterOracle
{
public:
    virtual auto GetFilterJob() const noexcept -> CfilterJob = 0;
    virtual auto GetHeaderJob() const noexcept -> CfheaderJob = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    auto Internal() const noexcept -> const internal::FilterOracle& final
    {
        return *this;
    }
    virtual auto LoadFilterOrResetTip(
        const cfilter::Type type,
        const block::Position& position,
        alloc::Default alloc) const noexcept -> GCS = 0;
    virtual auto ProcessBlock(const bitcoin::block::Block& block) const noexcept
        -> bool = 0;
    virtual auto ProcessBlock(
        cfilter::Type type,
        const bitcoin::block::Block& block,
        alloc::Default alloc) const noexcept -> GCS = 0;
    virtual auto ProcessSyncData(
        const block::Hash& prior,
        const Vector<block::Hash>& hashes,
        const network::p2p::Data& data) const noexcept -> void = 0;
    virtual auto Tip(const cfilter::Type type) const noexcept
        -> block::Position = 0;

    auto Internal() noexcept -> internal::FilterOracle& final { return *this; }
    virtual auto Start() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    ~FilterOracle() override = default;
};
}  // namespace opentxs::blockchain::node::internal

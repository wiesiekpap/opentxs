// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "api/client/blockchain/database/Database.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace client
{
struct GCS;
}  // namespace client
}  // namespace blockchain

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

namespace opentxs::blockchain::database
{
class Filters
{
public:
    using Common = api::client::blockchain::database::implementation::Database;
    using Parent = client::internal::FilterDatabase;
    using Hash = Parent::Hash;
    using Header = Parent::Header;
    using Filter = Parent::Filter;

    auto CurrentHeaderTip(const filter::Type type) const noexcept
        -> block::Position;
    auto CurrentTip(const filter::Type type) const noexcept -> block::Position;
    auto HaveFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> bool
    {
        return common_.HaveFilter(type, block.Bytes());
    }
    auto HaveFilterHeader(const filter::Type type, const block::Hash& block)
        const noexcept -> bool
    {
        return common_.HaveFilterHeader(type, block.Bytes());
    }
    auto LoadFilter(const filter::Type type, const ReadView block)
        const noexcept -> std::unique_ptr<const blockchain::client::GCS>
    {
        return common_.LoadFilter(type, block);
    }
    auto LoadFilterHash(const filter::Type type, const ReadView block)
        const noexcept -> Hash;
    auto LoadFilterHeader(const filter::Type type, const ReadView block)
        const noexcept -> Hash;
    auto SetHeaderTip(const filter::Type type, const block::Position& position)
        const noexcept -> bool;
    auto SetTip(const filter::Type type, const block::Position& position)
        const noexcept -> bool;
    auto StoreHeaders(
        const filter::Type type,
        const ReadView previous,
        const std::vector<Header> headers) const noexcept -> bool
    {
        return common_.StoreFilterHeaders(type, headers);
    }
    auto StoreFilters(
        const filter::Type type,
        const std::vector<Header>& headers,
        const std::vector<Filter>& filters,
        const block::Position& tip) const noexcept -> bool;
    auto StoreFilters(const filter::Type type, std::vector<Filter> filters)
        const noexcept -> bool
    {
        return common_.StoreFilters(type, filters);
    }

    Filters(
        const api::Core& api,
        const Common& common,
        const opentxs::storage::lmdb::LMDB& lmdb,
        const blockchain::Type chain) noexcept;

private:
    const api::Core& api_;
    const Common& common_;
    const opentxs::storage::lmdb::LMDB& lmdb_;
    const block::Position blank_position_;
    mutable std::mutex lock_;

    auto import_genesis(const blockchain::Type type) const noexcept -> void;
};
}  // namespace opentxs::blockchain::database

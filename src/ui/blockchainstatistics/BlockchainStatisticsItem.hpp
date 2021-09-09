// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/ui/BlockchainStatisticsItem.hpp"
#include "ui/base/Row.hpp"

class QVariant;

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal

class Manager;
}  // namespace client
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class BlockchainStatisticsItem;
}  // namespace ui
}  // namespace opentxs

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
namespace opentxs::ui::implementation
{
using BlockchainStatisticsItemRow =
    Row<BlockchainStatisticsRowInternal,
        BlockchainStatisticsInternalInterface,
        BlockchainStatisticsRowID>;

class BlockchainStatisticsItem final
    : public BlockchainStatisticsItemRow,
      public std::enable_shared_from_this<BlockchainStatisticsItem>
{
public:
    auto ActivePeers() const noexcept -> std::size_t final
    {
        return active_peers_.load();
    }
    auto Balance() const noexcept -> std::string final;
    auto BlockDownloadQueue() const noexcept -> std::size_t final
    {
        return blocks_.load();
    }
    auto Chain() const noexcept -> blockchain::Type final { return row_id_; }
    auto ConnectedPeers() const noexcept -> std::size_t final
    {
        return connected_peers_.load();
    }
    auto Filters() const noexcept -> Position final { return filter_.load(); }
    auto Headers() const noexcept -> Position final { return header_.load(); }
    auto Name() const noexcept -> std::string final { return name_; }

    BlockchainStatisticsItem(
        const BlockchainStatisticsInternalInterface& parent,
        const api::client::Manager& api,
        const BlockchainStatisticsRowID& rowID,
        const BlockchainStatisticsSortKey& sortKey,
        CustomData& custom) noexcept;

    ~BlockchainStatisticsItem() final;

private:
    const std::string name_;
    std::atomic<blockchain::block::Height> header_;
    std::atomic<blockchain::block::Height> filter_;
    std::atomic<std::size_t> connected_peers_;
    std::atomic<std::size_t> active_peers_;
    std::atomic<std::size_t> blocks_;
    std::atomic<blockchain::Amount> balance_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const BlockchainStatisticsSortKey&, CustomData& data) noexcept
        -> bool final;

    BlockchainStatisticsItem() = delete;
    BlockchainStatisticsItem(const BlockchainStatisticsItem&) = delete;
    BlockchainStatisticsItem(BlockchainStatisticsItem&&) = delete;
    auto operator=(const BlockchainStatisticsItem&)
        -> BlockchainStatisticsItem& = delete;
    auto operator=(BlockchainStatisticsItem&&)
        -> BlockchainStatisticsItem& = delete;
};
}  // namespace opentxs::ui::implementation
#pragma GCC diagnostic pop

template class opentxs::SharedPimpl<opentxs::ui::BlockchainStatisticsItem>;

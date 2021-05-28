// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/blockchainstatistics/BlockchainStatisticsItem.hpp"  // IWYU pragma: associated

#if OT_QT
#include <QObject>
#endif  // OT_QT
#include <memory>

#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/core/Log.hpp"
#if OT_QT
#include "opentxs/ui/qt/BlockchainStatistics.hpp"
#endif  // OT_QT
#include "ui/base/Widget.hpp"

// #define OT_METHOD "opentxs::ui::implementation::BlockchainStatisticsItem::"

namespace opentxs::factory
{
auto BlockchainStatisticsItem(
    const ui::implementation::BlockchainStatisticsInternalInterface& parent,
    const api::client::internal::Manager& api,
    const ui::implementation::BlockchainStatisticsRowID& rowID,
    const ui::implementation::BlockchainStatisticsSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::BlockchainStatisticsRowInternal>
{
    using ReturnType = ui::implementation::BlockchainStatisticsItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainStatisticsItem::BlockchainStatisticsItem(
    const BlockchainStatisticsInternalInterface& parent,
    const api::client::internal::Manager& api,
    const BlockchainStatisticsRowID& rowID,
    const BlockchainStatisticsSortKey& sortKey,
    CustomData& custom) noexcept
    : BlockchainStatisticsItemRow(parent, api, rowID, true)
    , name_(sortKey)
    , header_(extract_custom<blockchain::block::Height>(custom, 0))
    , filter_(extract_custom<blockchain::block::Height>(custom, 1))
    , connected_peers_(extract_custom<std::size_t>(custom, 2))
    , active_peers_(extract_custom<std::size_t>(custom, 3))
    , blocks_(extract_custom<std::size_t>(custom, 4))
    , balance_(extract_custom<blockchain::Amount>(custom, 5))
{
}

auto BlockchainStatisticsItem::Balance() const noexcept -> std::string
{
    return blockchain::internal::Format(row_id_, balance_.load());
}

#if OT_QT
auto BlockchainStatisticsItem::qt_data(const int column, int role)
    const noexcept -> QVariant
{
    switch (role) {
        case Qt::TextAlignmentRole: {
            switch (column) {
                case BlockchainStatisticsQt::NameColumn: {

                    return Qt::AlignLeft;
                }
                default: {

                    return Qt::AlignHCenter;
                }
            }
        }
        case Qt::DisplayRole: {
            switch (column) {
                case BlockchainStatisticsQt::NameColumn: {

                    return qt_data(column, BlockchainStatisticsQt::Name);
                }
                case BlockchainStatisticsQt::BalanceColumn: {

                    return qt_data(column, BlockchainStatisticsQt::Balance);
                }
                case BlockchainStatisticsQt::HeaderColumn: {

                    return qt_data(
                        column, BlockchainStatisticsQt::HeaderHeight);
                }
                case BlockchainStatisticsQt::FilterColumn: {

                    return qt_data(
                        column, BlockchainStatisticsQt::FilterHeight);
                }
                case BlockchainStatisticsQt::ConnectedPeerColumn: {

                    return qt_data(
                        column, BlockchainStatisticsQt::ConnectedPeerCount);
                }
                case BlockchainStatisticsQt::ActivePeerColumn: {

                    return qt_data(
                        column, BlockchainStatisticsQt::ActivePeerCount);
                }
                case BlockchainStatisticsQt::BlockQueueColumn: {

                    return qt_data(column, BlockchainStatisticsQt::BlockQueue);
                }
                default: {
                }
            }
        } break;
        case BlockchainStatisticsQt::Balance: {
            return Balance().c_str();
        }
        case BlockchainStatisticsQt::BlockQueue: {
            return static_cast<int>(BlockDownloadQueue());
        }
        case BlockchainStatisticsQt::Chain: {
            return static_cast<int>(static_cast<std::uint32_t>(Chain()));
        }
        case BlockchainStatisticsQt::FilterHeight: {
            return static_cast<int>(Filters());
        }
        case BlockchainStatisticsQt::HeaderHeight: {
            return static_cast<int>(Headers());
        }
        case BlockchainStatisticsQt::Name: {
            return Name().c_str();
        }
        case BlockchainStatisticsQt::ActivePeerCount: {
            return static_cast<int>(ActivePeers());
        }
        case BlockchainStatisticsQt::ConnectedPeerCount: {
            return static_cast<int>(ConnectedPeers());
        }
        default: {
        }
    }

    return {};
}
#endif

auto BlockchainStatisticsItem::reindex(
    const BlockchainStatisticsSortKey& key,
    CustomData& custom) noexcept -> bool
{
    OT_ASSERT(name_ == key);

    const auto header = extract_custom<blockchain::block::Height>(custom, 0);
    const auto filter = extract_custom<blockchain::block::Height>(custom, 1);
    const auto connected = extract_custom<std::size_t>(custom, 2);
    const auto active = extract_custom<std::size_t>(custom, 3);
    const auto blocks = extract_custom<std::size_t>(custom, 4);
    const auto balance = extract_custom<blockchain::Amount>(custom, 5);
    const auto oldHeader = header_.exchange(header);
    const auto oldFilter = filter_.exchange(filter);
    const auto oldConnected = connected_peers_.exchange(connected);
    const auto oldActive = active_peers_.exchange(active);
    const auto oldBlocks = blocks_.exchange(blocks);
    const auto oldBalance = balance_.exchange(balance);

    const auto changed = (header != oldHeader) || (filter != oldFilter) ||
                         (connected != oldConnected) || (active != oldActive) ||
                         (blocks != oldBlocks) || (balance != oldBalance);

    return changed;
}

BlockchainStatisticsItem::~BlockchainStatisticsItem() = default;
}  // namespace opentxs::ui::implementation

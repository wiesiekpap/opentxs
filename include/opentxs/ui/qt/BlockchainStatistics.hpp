// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_BLOCKCHAINSTATISTICSQT_HPP
#define OPENTXS_UI_BLOCKCHAINSTATISTICSQT_HPP

#include <QIdentityProxyModel>
#include <QVariant>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

class QModelIndex;

namespace opentxs
{
namespace ui
{
namespace implementation
{
class BlockchainStatistics;
}  // namespace implementation

class BlockchainStatisticsQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::BlockchainStatisticsQt final
    : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    void updated() const;

public:
    // Seven columns when used in a table view
    //
    // All data is available in a list view via the user roles defined below:
    enum Roles {
        Balance = Qt::UserRole + 0,     // int, opentxs::blockchain::Amount
        BlockQueue = Qt::UserRole + 1,  // int, std::size_t
        Chain = Qt::UserRole + 2,       // int, opentxs::blockchain::Type
        FilterHeight =
            Qt::UserRole + 3,  // int, opentxs::blockchain::block::Height
        HeaderHeight =
            Qt::UserRole + 4,     // int, opentxs::blockchain::block::Height
        Name = Qt::UserRole + 5,  // QString
        ActivePeerCount = Qt::UserRole + 6,     // int, std::size_t
        ConnectedPeerCount = Qt::UserRole + 7,  // int, std::size_t
    };
    enum Columns {
        NameColumn = 0,
        BalanceColumn = 1,
        HeaderColumn = 2,
        FilterColumn = 3,
        ConnectedPeerColumn = 4,
        ActivePeerColumn = 5,
        BlockQueueColumn = 6,
    };

    auto headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole) const -> QVariant final;

    BlockchainStatisticsQt(
        implementation::BlockchainStatistics& parent) noexcept;

    ~BlockchainStatisticsQt() final;

private:
    implementation::BlockchainStatistics& parent_;

    auto notify() const noexcept -> void;

    BlockchainStatisticsQt(const BlockchainStatisticsQt&) = delete;
    BlockchainStatisticsQt(BlockchainStatisticsQt&&) = delete;
    BlockchainStatisticsQt& operator=(const BlockchainStatisticsQt&) = delete;
    BlockchainStatisticsQt& operator=(BlockchainStatisticsQt&&) = delete;
};
#endif

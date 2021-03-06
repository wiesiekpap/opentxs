// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_BLOCKCHAINSELECTIONQT_HPP
#define OPENTXS_UI_BLOCKCHAINSELECTIONQT_HPP

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
class BlockchainSelection;
}  // namespace implementation

class BlockchainSelectionQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::BlockchainSelectionQt final
    : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int enabledCount READ enabledCount NOTIFY enabledChanged)

signals:
    void chainEnabled(int chain);
    void chainDisabled(int chain);
    void enabledChanged(int enabledCount);
    void updated() const;

public:
    // One column
    //
    // Qt::DisplayRole and Qt::CheckStateRole are implemented in addition to the
    // roles below:
    enum Roles {
        TypeRole = Qt::UserRole + 0,   // int, opentxs::blockchain::Type
        IsTestnet = Qt::UserRole + 1,  // bool
    };

    auto enabledCount() const noexcept -> int;
    auto flags(const QModelIndex& index) const -> Qt::ItemFlags final;

    /// chain is an opentxs::blockchain::Type, retrievable as TypeRole
    Q_INVOKABLE bool disableChain(const int chain) noexcept;
    /// chain is an opentxs::blockchain::Type, retrievable as TypeRole
    Q_INVOKABLE bool enableChain(const int chain) noexcept;
    auto setData(const QModelIndex& index, const QVariant& value, int role)
        -> bool final;

    BlockchainSelectionQt(implementation::BlockchainSelection& parent) noexcept;

    ~BlockchainSelectionQt() final;

private:
    implementation::BlockchainSelection& parent_;

    auto notify() const noexcept -> void;

    auto init() noexcept -> void;

    BlockchainSelectionQt(const BlockchainSelectionQt&) = delete;
    BlockchainSelectionQt(BlockchainSelectionQt&&) = delete;
    BlockchainSelectionQt& operator=(const BlockchainSelectionQt&) = delete;
    BlockchainSelectionQt& operator=(BlockchainSelectionQt&&) = delete;
};
#endif

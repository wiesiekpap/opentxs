// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_BLOCKCHAINSELECTIONQT_HPP
#define OPENTXS_UI_BLOCKCHAINSELECTIONQT_HPP

#include <QIdentityProxyModel>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

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

signals:
    void updated() const;

public:
    // User roles return the same data for all columns
    //
    // TypeRole: int (blockchain::Type)
    //
    // Qt::DisplayRole, NameColumn: QString
    // Qt::DisplayRole, EnabledColumn: bool
    // Qt::DisplayRole, TestnetColumn: bool

    enum Columns {
        NameColumn = 0,
        EnabledColumn = 1,
        TestnetColumn = 2,
    };
    enum Roles {
        TypeRole = Qt::UserRole + 0,
    };

    Q_INVOKABLE bool disableChain(const int chain) const noexcept;
    Q_INVOKABLE bool enableChain(const int chain) const noexcept;

    BlockchainSelectionQt(implementation::BlockchainSelection& parent) noexcept;

    ~BlockchainSelectionQt() final = default;

private:
    implementation::BlockchainSelection& parent_;

    void notify() const noexcept;

    BlockchainSelectionQt(const BlockchainSelectionQt&) = delete;
    BlockchainSelectionQt(BlockchainSelectionQt&&) = delete;
    BlockchainSelectionQt& operator=(const BlockchainSelectionQt&) = delete;
    BlockchainSelectionQt& operator=(BlockchainSelectionQt&&) = delete;
};
#endif

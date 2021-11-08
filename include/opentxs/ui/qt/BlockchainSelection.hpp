// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep
#include "opentxs/ui/qt/Model.hpp"

class QModelIndex;
class QObject;

namespace opentxs
{
namespace ui
{
namespace internal
{
struct BlockchainSelection;
}  // namespace internal

class BlockchainSelectionQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::BlockchainSelectionQt final : public qt::Model
{
    Q_OBJECT
    Q_PROPERTY(int enabledCount READ enabledCount NOTIFY enabledChanged)

signals:
    void chainEnabled(int chain);
    void chainDisabled(int chain);
    void enabledChanged(int enabledCount);

public:
    /// chain is an opentxs::blockchain::Type, retrievable as TypeRole
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE bool disableChain(const int chain) noexcept;
    /// chain is an opentxs::blockchain::Type, retrievable as TypeRole
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE bool enableChain(const int chain) noexcept;

public:
    enum Roles {
        NameRole = Qt::UserRole + 0,   // QString
        TypeRole = Qt::UserRole + 1,   // int, opentxs::blockchain::Type
        IsEnabled = Qt::UserRole + 2,  // bool
        IsTestnet = Qt::UserRole + 3,  // bool
    };
    enum Columns {
        NameColumn = 0,
    };

    auto enabledCount() const noexcept -> int;
    auto flags(const QModelIndex& index) const -> Qt::ItemFlags final;

    auto setData(const QModelIndex& index, const QVariant& value, int role)
        -> bool final;

    BlockchainSelectionQt(internal::BlockchainSelection& parent) noexcept;

    ~BlockchainSelectionQt() final;

private:
    struct Imp;

    Imp* imp_;

    BlockchainSelectionQt(const BlockchainSelectionQt&) = delete;
    BlockchainSelectionQt(BlockchainSelectionQt&&) = delete;
    BlockchainSelectionQt& operator=(const BlockchainSelectionQt&) = delete;
    BlockchainSelectionQt& operator=(BlockchainSelectionQt&&) = delete;
};

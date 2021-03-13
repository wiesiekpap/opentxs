// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_DISPLAYSCALE_HPP
#define OPENTXS_UI_DISPLAYSCALE_HPP

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

class QObject;

namespace opentxs
{
namespace display
{
class Definition;
}  // namespace display

namespace ui
{
class DisplayScaleQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::DisplayScaleQt final
    : public QAbstractListModel
{
    Q_OBJECT

public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)
        const final;

    DisplayScaleQt(const display::Definition&) noexcept;
    DisplayScaleQt(const DisplayScaleQt&) noexcept;

    ~DisplayScaleQt() final = default;

private:
    const display::Definition& data_;

    DisplayScaleQt() = delete;
    DisplayScaleQt(DisplayScaleQt&&) = delete;
    DisplayScaleQt& operator=(const DisplayScaleQt&) = delete;
    DisplayScaleQt& operator=(DisplayScaleQt&&) = delete;
};
#endif

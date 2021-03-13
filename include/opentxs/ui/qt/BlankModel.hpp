// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_BLANKMODELQT_HPP
#define OPENTXS_UI_BLANKMODELQT_HPP

#include <QIdentityProxyModel>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

namespace opentxs
{
namespace ui
{
struct OPENTXS_EXPORT BlankModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    int columnCount(const QModelIndex& = QModelIndex()) const noexcept final
    {
        return static_cast<int>(columns_);
    }
    QVariant data(const QModelIndex&, int = Qt::DisplayRole)
        const noexcept final
    {
        return {};
    }
    QModelIndex index(int, int, const QModelIndex& = QModelIndex())
        const noexcept final
    {
        return {};
    }
    QModelIndex parent(const QModelIndex&) const noexcept final { return {}; }
    int rowCount(const QModelIndex& = QModelIndex()) const noexcept final
    {
        return 0;
    }

    BlankModel(const std::size_t columns)
        : columns_(columns)
    {
        assert(columns_ <= std::numeric_limits<int>::max());
    }

private:
    const std::size_t columns_;

    BlankModel() = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

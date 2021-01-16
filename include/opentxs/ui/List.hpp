// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_LIST_HPP
#define OPENTXS_UI_LIST_HPP

#include <cassert>
#include <limits>

#ifndef Q_MOC_RUN
#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/ui/Widget.hpp"

#ifdef SWIG
// clang-format off
%rename(UIList) opentxs::ui::List;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class List : virtual public Widget
{
public:
    OPENTXS_EXPORT ~List() override = default;

protected:
    List() noexcept = default;

private:
    List(const List&) = delete;
    List(List&&) = delete;
    List& operator=(const List&) = delete;
    List& operator=(List&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
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
#endif

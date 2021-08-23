// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_UNITLISTQT_HPP
#define OPENTXS_UI_UNITLISTQT_HPP

#include <QObject>
#include <QString>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep
#include "opentxs/ui/qt/Model.hpp"

class QObject;

namespace opentxs
{
namespace ui
{
namespace internal
{
struct UnitList;
}  // namespace internal

class UnitListQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::UnitListQt final : public qt::Model
{
    Q_OBJECT
public:
    enum Columns {
        UnitNameColumn = 0,
    };
    enum Roles {
        UnitIDRole = Qt::UserRole,
    };

    UnitListQt(internal::UnitList& parent) noexcept;

    ~UnitListQt() final;

private:
    struct Imp;

    Imp* imp_;

    UnitListQt(const UnitListQt&) = delete;
    UnitListQt(UnitListQt&&) = delete;
    UnitListQt& operator=(const UnitListQt&) = delete;
    UnitListQt& operator=(UnitListQt&&) = delete;
};
#endif

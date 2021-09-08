// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PAYABLELISTQT_HPP
#define OPENTXS_UI_PAYABLELISTQT_HPP

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
struct PayableList;
}  // namespace internal

class PayableListQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::PayableListQt final : public qt::Model
{
    Q_OBJECT
public:
    // Table layout: name, payment code
    enum Roles {
        ContactIDRole = Qt::UserRole + 0,
        SectionRole = Qt::UserRole + 1,
    };

    PayableListQt(internal::PayableList& parent) noexcept;

    ~PayableListQt() final;

private:
    struct Imp;

    Imp* imp_;

    PayableListQt(const PayableListQt&) = delete;
    PayableListQt(PayableListQt&&) = delete;
    PayableListQt& operator=(const PayableListQt&) = delete;
    PayableListQt& operator=(PayableListQt&&) = delete;
};
#endif

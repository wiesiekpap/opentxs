// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PAYABLELISTQT_HPP
#define OPENTXS_UI_PAYABLELISTQT_HPP

#include <QIdentityProxyModel>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

namespace opentxs
{
namespace ui
{
namespace implementation
{
class PayableList;
}  // namespace implementation

class PayableListQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::PayableListQt final
    : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    void updated() const;

public:
    // Table layout: name, payment code
    enum Roles {
        ContactIDRole = Qt::UserRole + 0,
        SectionRole = Qt::UserRole + 1,
    };

    PayableListQt(implementation::PayableList& parent) noexcept;

    ~PayableListQt() final = default;

private:
    implementation::PayableList& parent_;

    void notify() const noexcept;

    PayableListQt(const PayableListQt&) = delete;
    PayableListQt(PayableListQt&&) = delete;
    PayableListQt& operator=(const PayableListQt&) = delete;
    PayableListQt& operator=(PayableListQt&&) = delete;
};
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYSUMMARYQT_HPP
#define OPENTXS_UI_ACTIVITYSUMMARYQT_HPP

#include <QIdentityProxyModel>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

namespace opentxs
{
namespace ui
{
namespace implementation
{
class ActivitySummary;
}  // namespace implementation

class ActivitySummaryQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::ActivitySummaryQt final
    : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    void updated() const;

public:
    ActivitySummaryQt(implementation::ActivitySummary& parent) noexcept;

    ~ActivitySummaryQt() final = default;

private:
    implementation::ActivitySummary& parent_;

    void notify() const noexcept;

    ActivitySummaryQt(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt(ActivitySummaryQt&&) = delete;
    ActivitySummaryQt& operator=(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt& operator=(ActivitySummaryQt&&) = delete;
};
#endif

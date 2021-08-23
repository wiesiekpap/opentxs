// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYSUMMARYQT_HPP
#define OPENTXS_UI_ACTIVITYSUMMARYQT_HPP

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
struct ActivitySummary;
}  // namespace internal

class ActivitySummaryQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::ActivitySummaryQt final : public qt::Model
{
    Q_OBJECT

public:
    ActivitySummaryQt(internal::ActivitySummary& parent) noexcept;

    ~ActivitySummaryQt() final;

private:
    struct Imp;

    Imp* imp_;

    ActivitySummaryQt(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt(ActivitySummaryQt&&) = delete;
    ActivitySummaryQt& operator=(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt& operator=(ActivitySummaryQt&&) = delete;
};
#endif

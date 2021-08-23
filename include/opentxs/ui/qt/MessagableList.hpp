// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_MESSAGABLELISTQT_HPP
#define OPENTXS_UI_MESSAGABLELISTQT_HPP

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
struct MessagableList;
}  // namespace internal

class MessagableListQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::MessagableListQt final : public qt::Model
{
    Q_OBJECT
public:
    // List layout
    enum Roles {
        ContactIDRole = Qt::UserRole + 0,
        SectionRole = Qt::UserRole + 1,
    };

    MessagableListQt(internal::MessagableList& parent) noexcept;

    ~MessagableListQt() final;

private:
    struct Imp;

    Imp* imp_;

    MessagableListQt() = delete;
    MessagableListQt(const MessagableListQt&) = delete;
    MessagableListQt(MessagableListQt&&) = delete;
    MessagableListQt& operator=(const MessagableListQt&) = delete;
    MessagableListQt& operator=(MessagableListQt&&) = delete;
};
#endif

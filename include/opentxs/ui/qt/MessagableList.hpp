// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_MESSAGABLELISTQT_HPP
#define OPENTXS_UI_MESSAGABLELISTQT_HPP

#include <QIdentityProxyModel>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

namespace opentxs
{
namespace ui
{
namespace implementation
{
class MessagableList;
}  // namespace implementation

class MessagableListQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::MessagableListQt final
    : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    void updated() const;

public:
    // List layout
    enum Roles {
        ContactIDRole = Qt::UserRole + 0,
        SectionRole = Qt::UserRole + 1,
    };

    MessagableListQt(implementation::MessagableList& parent) noexcept;

    ~MessagableListQt() final = default;

private:
    implementation::MessagableList& parent_;

    void notify() const noexcept;

    MessagableListQt() = delete;
    MessagableListQt(const MessagableListQt&) = delete;
    MessagableListQt(MessagableListQt&&) = delete;
    MessagableListQt& operator=(const MessagableListQt&) = delete;
    MessagableListQt& operator=(MessagableListQt&&) = delete;
};
#endif

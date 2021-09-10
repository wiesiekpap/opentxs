// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTLISTQT_HPP
#define OPENTXS_UI_CONTACTLISTQT_HPP

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
struct ContactList;
}  // namespace internal

class ContactListQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::ContactListQt final : public qt::Model
{
    Q_OBJECT

public:
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QString addContact(
        const QString& label,
        const QString& paymentCode = "",
        const QString& nymID = "") const noexcept;

public:
    enum Roles {
        IDRole = Qt::UserRole + 0,       // QString
        NameRole = Qt::UserRole + 1,     // QString
        ImageRole = Qt::UserRole + 2,    // QPixmap
        SectionRole = Qt::UserRole + 3,  // QString
    };
    // This model is designed to be used in a list view
    enum Columns {
        NameColumn = 0,
    };

    ContactListQt(internal::ContactList& parent) noexcept;

    ~ContactListQt() final;

private:
    struct Imp;

    Imp* imp_;

    ContactListQt() = delete;
    ContactListQt(const ContactListQt&) = delete;
    ContactListQt(ContactListQt&&) = delete;
    ContactListQt& operator=(const ContactListQt&) = delete;
    ContactListQt& operator=(ContactListQt&&) = delete;
};
#endif

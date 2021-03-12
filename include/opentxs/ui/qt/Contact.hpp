// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTQT_HPP
#define OPENTXS_UI_CONTACTQT_HPP

#include <QIdentityProxyModel>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

namespace opentxs
{
namespace ui
{
namespace implementation
{
class Contact;
}  // namespace implementation

class ContactQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::ContactQt final : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName NOTIFY updated)
    Q_PROPERTY(QString contactID READ contactID NOTIFY updated)
    Q_PROPERTY(QString paymentCode READ paymentCode NOTIFY updated)

signals:
    void updated() const;

public:
    // Tree layout
    QString displayName() const noexcept;
    QString contactID() const noexcept;
    QString paymentCode() const noexcept;

    ContactQt(implementation::Contact& parent) noexcept;

    ~ContactQt() final = default;

private:
    implementation::Contact& parent_;

    void notify() const noexcept;

    ContactQt() = delete;
    ContactQt(const ContactQt&) = delete;
    ContactQt(ContactQt&&) = delete;
    ContactQt& operator=(const ContactQt&) = delete;
    ContactQt& operator=(ContactQt&&) = delete;
};
#endif

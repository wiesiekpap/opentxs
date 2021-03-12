// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PROFILEQT_HPP
#define OPENTXS_UI_PROFILEQT_HPP

#include <QIdentityProxyModel>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

namespace opentxs
{
namespace ui
{
namespace implementation
{
class Profile;
}  // namespace implementation

class ProfileQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::ProfileQt final : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName NOTIFY updated)
    Q_PROPERTY(QString nymID READ nymID NOTIFY updated)
    Q_PROPERTY(QString paymentCode READ paymentCode NOTIFY updated)

signals:
    void updated() const;

public:
    // Tree layout
    QString displayName() const noexcept;
    QString nymID() const noexcept;
    QString paymentCode() const noexcept;

    ProfileQt(implementation::Profile& parent) noexcept;

    ~ProfileQt() final = default;

private:
    implementation::Profile& parent_;

    void notify() const noexcept;

    ProfileQt(const ProfileQt&) = delete;
    ProfileQt(ProfileQt&&) = delete;
    ProfileQt& operator=(const ProfileQt&) = delete;
    ProfileQt& operator=(ProfileQt&&) = delete;
};
#endif

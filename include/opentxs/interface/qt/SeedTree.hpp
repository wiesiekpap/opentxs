// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QString>

#include "opentxs/interface/qt/Model.hpp"

class QObject;

namespace opentxs
{
namespace ui
{
namespace internal
{
struct SeedTree;
}  // namespace internal

class SeedTreeQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::SeedTreeQt final : public qt::Model
{
    Q_OBJECT

public:
    enum Roles {
        SeedIDRole = Qt::UserRole + 0,    // QString
        SeedNameRole = Qt::UserRole + 1,  // QString
        SeedTypeRole = Qt::UserRole + 2,  // int (crypto::SeedStyle)
        NymIDRole = Qt::UserRole + 3,     // QString
        NymNameRole = Qt::UserRole + 4,   // QString
        NymIndexRole = Qt::UserRole + 5,  // unsigned long long
    };
    enum Columns {
        NameColumn = 0,
    };

    SeedTreeQt(internal::SeedTree& parent) noexcept;

    ~SeedTreeQt() final;

private:
    struct Imp;

    Imp* imp_;

    SeedTreeQt(const SeedTreeQt&) = delete;
    SeedTreeQt(SeedTreeQt&&) = delete;
    SeedTreeQt& operator=(const SeedTreeQt&) = delete;
    SeedTreeQt& operator=(SeedTreeQt&&) = delete;
};

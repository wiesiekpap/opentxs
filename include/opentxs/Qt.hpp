// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_QT_HPP
#define OPENTXS_QT_HPP

// IWYU pragma: begin_exports
#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#if OT_QT
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#ifndef Q_MOC_RUN
#include <QtCore/QAbstractItemModel>
#include <QtCore/QAbstractListModel>
#include <QtCore/QDateTime>
#include <QtCore/QIdentityProxyModel>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QValidator>
#endif
#pragma GCC diagnostic pop
#endif
#endif
// IWYU pragma: end_exports

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_BLOCKCHAINACCOUNTSTATUSQT_HPP
#define OPENTXS_UI_BLOCKCHAINACCOUNTSTATUSQT_HPP

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
struct BlockchainAccountStatus;
}  // namespace internal

class BlockchainAccountStatusQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::BlockchainAccountStatusQt final
    : public qt::Model
{
    Q_OBJECT
    Q_PROPERTY(QString nym READ getNym CONSTANT)
    Q_PROPERTY(int chain READ getChain CONSTANT)

public:
    enum Roles {
        NameRole = Qt::UserRole + 0,            // QString
        SourceIDRole = Qt::UserRole + 1,        // QString
        SubaccountIDRole = Qt::UserRole + 2,    // QString
        SubaccountTypeRole = Qt::UserRole + 3,  // int
        SubchainTypeRole = Qt::UserRole + 4,    // int
        ProgressRole = Qt::UserRole + 5,        // QString
    };
    // This model is designed to be used in a tree view
    enum Columns {
        NameColumn = 0,
    };

    auto getNym() const noexcept -> QString;
    auto getChain() const noexcept -> int;

    BlockchainAccountStatusQt(
        internal::BlockchainAccountStatus& parent) noexcept;

    ~BlockchainAccountStatusQt() final;

private:
    struct Imp;

    Imp* imp_;

    BlockchainAccountStatusQt() = delete;
    BlockchainAccountStatusQt(const BlockchainAccountStatusQt&) = delete;
    BlockchainAccountStatusQt(BlockchainAccountStatusQt&&) = delete;
    BlockchainAccountStatusQt& operator=(const BlockchainAccountStatusQt&) =
        delete;
    BlockchainAccountStatusQt& operator=(BlockchainAccountStatusQt&&) = delete;
};
#endif

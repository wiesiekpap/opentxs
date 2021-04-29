// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_DESTINATIONVALIDATOR_HPP
#define OPENTXS_UI_DESTINATIONVALIDATOR_HPP

#include <QObject>
#include <QString>
#include <QValidator>
#include <cstdint>
#include <memory>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

class QObject;

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace ui
{
namespace implementation
{
class AccountActivity;
}  // namespace implementation

class DestinationValidator;
}  // namespace ui

class Identifier;
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::DestinationValidator final : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(QString details READ getDetails NOTIFY detailsChanged)

signals:
    void detailsChanged(const QString&);

public:
    struct Imp;

    QString getDetails() const;
    void fixup(QString& input) const final;
    State validate(QString& input, int& pos) const final;

    OPENTXS_NO_EXPORT DestinationValidator(
        const api::client::Manager&,
        std::int8_t,
        const Identifier&,
        implementation::AccountActivity&) noexcept;

    OPENTXS_NO_EXPORT ~DestinationValidator() final;

private:
    std::unique_ptr<Imp> imp_;

    DestinationValidator() = delete;
    DestinationValidator(const DestinationValidator&) = delete;
    DestinationValidator(DestinationValidator&&) = delete;
    DestinationValidator& operator=(const DestinationValidator&) = delete;
    DestinationValidator& operator=(DestinationValidator&&) = delete;
};
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_SEEDVALIDATOR_HPP
#define OPENTXS_UI_SEEDVALIDATOR_HPP

#include <QObject>
#include <QString>
#include <QValidator>
#include <QVector>
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
class SeedValidator;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::SeedValidator final : public QValidator
{
    Q_OBJECT

public:
    using Phrase = QVector<QString>;

    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE bool checkPhrase(const Phrase& phrase) const;

public:
    void fixup(QString& input) const final;
    State validate(QString& input, int& pos) const final;

    SeedValidator(
        const api::client::Manager&,
        std::uint8_t,
        std::uint8_t) noexcept;

    ~SeedValidator() final;

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    SeedValidator() = delete;
    SeedValidator(const SeedValidator&) = delete;
    SeedValidator(SeedValidator&&) = delete;
    SeedValidator& operator=(const SeedValidator&) = delete;
    SeedValidator& operator=(SeedValidator&&) = delete;
};
#endif

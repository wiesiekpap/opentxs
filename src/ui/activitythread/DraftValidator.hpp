// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QObject>
#include <QString>
#include <QValidator>
#include <memory>

class QObject;

namespace opentxs
{
namespace ui
{
namespace implementation
{
class ActivityThread;
class DraftValidator;
}  // namespace implementation
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
class DraftValidator final : public QValidator
{
    Q_OBJECT

public:
    void fixup(QString& input) const final;
    State validate(QString& input, int& pos) const final;

    DraftValidator(implementation::ActivityThread& parent) noexcept;

    ~DraftValidator() final = default;

private:
    implementation::ActivityThread& parent_;

    DraftValidator() = delete;
    DraftValidator(const DraftValidator&) = delete;
    DraftValidator(DraftValidator&&) = delete;
    DraftValidator& operator=(const DraftValidator&) = delete;
    DraftValidator& operator=(DraftValidator&&) = delete;
};
}  // namespace opentxs::ui::implementation

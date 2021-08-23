// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QString>
#include <QValidator>
#include <memory>

#include "1_Internal.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/ui/qt/DestinationValidator.hpp"
#include "ui/accountactivity/AccountActivity.hpp"

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
}  // namespace ui

class Identifier;
}  // namespace opentxs

namespace opentxs::ui
{
struct DestinationValidator::Imp {
    using Parent = implementation::AccountActivity;

    static auto Blockchain(
        const api::client::Manager& api,
        DestinationValidator& main,
        const Identifier& account,
        Parent& parent) noexcept -> std::unique_ptr<Imp>;
    static auto Custodial(
        const api::client::Manager& api,
        Parent& parent) noexcept -> std::unique_ptr<Imp>;

    virtual auto fixup(QString& input) const -> void = 0;
    virtual auto getDetails() const -> QString = 0;
    virtual auto validate(QString& input, int& pos) const
        -> QValidator::State = 0;

    virtual ~Imp() = default;

protected:
    static auto strip_invalid(QString& input, bool cashaddr = false) noexcept
        -> void;
};
}  // namespace opentxs::ui

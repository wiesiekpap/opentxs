// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "1_Internal.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/ui/DestinationValidator.hpp"

namespace opentxs
{
namespace implementation
{
namespace implementation
{
class AccountActivity;
}  // namespace implementation
}  // namespace implementation
}  // namespace opentxs

namespace opentxs::ui
{
struct DestinationValidator::Imp {
    using Parent = implementation::AccountActivity;

    static auto Blockchain(
        const api::client::Manager& api,
        DestinationValidator& main,
        blockchain::Type chain,
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

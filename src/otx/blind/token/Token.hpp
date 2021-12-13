// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/otx/blind/Token.hpp"
#include "opentxs/otx/blind/Token.hpp"

namespace opentxs::otx::blind
{
class Token::Imp : virtual public internal::Token
{
public:
    virtual auto clone() const noexcept -> Imp*
    {
        return std::make_unique<Imp>(*this).release();
    }
    auto ID(const PasswordPrompt&) const -> std::string override { return {}; }
    auto IsSpent(const PasswordPrompt&) const -> bool override { return {}; }
    virtual auto IsValid() const noexcept -> bool { return false; }
    auto Notary() const -> const identifier::Server& override;
    auto Owner() const noexcept -> internal::Purse& override;
    auto Series() const -> MintSeries override { return {}; }
    auto State() const -> blind::TokenState override { return {}; }
    auto Type() const -> blind::CashType override { return {}; }
    auto Unit() const -> const identifier::UnitDefinition& override;
    auto ValidFrom() const -> Time override { return {}; }
    auto ValidTo() const -> Time override { return {}; }
    auto Value() const -> Denomination override { return {}; }
    auto Serialize(proto::Token&) const noexcept -> bool override { return {}; }

    auto ChangeOwner(
        blind::internal::Purse&,
        blind::internal::Purse&,
        const PasswordPrompt&) -> bool override
    {
        return {};
    }
    auto MarkSpent(const PasswordPrompt&) -> bool override { return {}; }
    auto Process(
        const identity::Nym&,
        const blind::Mint&,
        const PasswordPrompt&) -> bool override
    {
        return {};
    }

    Imp() noexcept {}
    Imp(const Imp&) noexcept {}
    Imp(Imp&&) noexcept {}

    ~Imp() override = default;
};
}  // namespace opentxs::otx::blind

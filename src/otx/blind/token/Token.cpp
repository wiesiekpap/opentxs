// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "1_Internal.hpp"               // IWYU pragma: associated
#include "opentxs/otx/blind/Token.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>
#include <string>

#include "internal/core/identifier/Factory.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/otx/blind/CashType.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "otx/blind/purse/Purse.hpp"
#include "otx/blind/token/Token.hpp"
#include "serialization/protobuf/Token.pb.h"

namespace opentxs::factory
{
using ReturnType = otx::blind::Token;

auto Token(const ReturnType& token, otx::blind::internal::Purse& purse) noexcept
    -> ReturnType
{
    const auto type = token.Type();

    switch (type) {
        case otx::blind::CashType::Lucre: {

            return TokenLucre(token, purse);
        }
        default: {
            LogError()("opentxs::factory::")(__func__)(
                ": unknown or unsupported token type: ")(opentxs::print(type))
                .Flush();

            return {};
        }
    }
}

auto Token(
    const api::Session& api,
    otx::blind::internal::Purse& purse,
    const proto::Token& serialized) noexcept -> ReturnType
{
    const auto type = translate(serialized.type());

    switch (type) {
        case otx::blind::CashType::Lucre: {

            return TokenLucre(api, purse, serialized);
        }
        default: {
            LogError()("opentxs::factory::")(__func__)(
                ": unknown or unsupported token type: ")(opentxs::print(type))
                .Flush();

            return {};
        }
    }
}

auto Token(
    const api::Session& api,
    const identity::Nym& owner,
    const otx::blind::Mint& mint,
    const otx::blind::Denomination value,
    otx::blind::internal::Purse& purse,
    const opentxs::PasswordPrompt& reason) noexcept -> ReturnType
{
    const auto type = purse.Type();

    switch (type) {
        case otx::blind::CashType::Lucre: {

            return TokenLucre(api, owner, mint, value, purse, reason);
        }
        default: {
            LogError()("opentxs::factory::")(__func__)(
                ": unknown or unsupported token type: ")(opentxs::print(type))
                .Flush();

            return {};
        }
    }
}
}  // namespace opentxs::factory

namespace opentxs::otx::blind
{
auto Token::Imp::Notary() const -> const identifier::Server&
{
    static const auto id = factory::IdentifierNotary();

    return *id;
}

auto Token::Imp::Owner() const noexcept -> internal::Purse&
{
    static auto blank = Purse::Imp{};

    return blank;
}

auto Token::Imp::Unit() const -> const identifier::UnitDefinition&
{
    static const auto id = factory::IdentifierUnit();

    return *id;
}
}  // namespace opentxs::otx::blind

namespace opentxs::otx::blind
{
auto swap(Token& lhs, Token& rhs) noexcept -> void { lhs.swap(rhs); }

Token::Token(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Token::Token() noexcept
    : Token(std::make_unique<Imp>().release())
{
}

Token::Token(const Token& rhs) noexcept
    : Token(rhs.imp_->clone())
{
}

Token::Token(Token&& rhs) noexcept
    : Token()
{
    swap(rhs);
}

Token::operator bool() const noexcept { return imp_->IsValid(); }

auto Token::ID(const PasswordPrompt& reason) const -> std::string
{
    return imp_->ID(reason);
}

auto Token::Internal() const noexcept -> const internal::Token&
{
    return *imp_;
}

auto Token::Internal() noexcept -> internal::Token& { return *imp_; }

auto Token::IsSpent(const PasswordPrompt& reason) const -> bool
{
    return imp_->IsSpent(reason);
}

auto Token::Notary() const -> const identifier::Server&
{
    return imp_->Notary();
}

auto Token::operator=(const Token& rhs) noexcept -> Token&
{
    auto old = std::unique_ptr<Imp>(imp_);
    imp_ = rhs.imp_->clone();

    return *this;
}

auto Token::operator=(Token&& rhs) noexcept -> Token&
{
    swap(rhs);

    return *this;
}

auto Token::Series() const -> MintSeries { return imp_->Series(); }

auto Token::State() const -> blind::TokenState { return imp_->State(); }

auto Token::swap(Token& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

auto Token::Type() const -> blind::CashType { return imp_->Type(); }

auto Token::Unit() const -> const identifier::UnitDefinition&
{
    return imp_->Unit();
}

auto Token::ValidFrom() const -> Time { return imp_->ValidFrom(); }

auto Token::ValidTo() const -> Time { return imp_->ValidTo(); }

auto Token::Value() const -> Denomination { return imp_->Value(); }

Token::~Token()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::otx::blind

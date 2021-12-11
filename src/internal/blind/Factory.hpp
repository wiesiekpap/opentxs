// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blind/CashType.hpp"
// IWYU pragma: no_include "opentxs/blind/PurseType.hpp"
// IWYU pragma: no_include "opentxs/blind/TokenState.hpp"

#pragma once

#include "opentxs/blind/Types.hpp"
#include "opentxs/blind/Token.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blind
{
class Mint;
class Token;
class Purse;
}  // namespace blind

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
class Server;
}  // namespace context
}  // namespace otx

namespace proto
{
class Purse;
class Token;
}  // namespace proto

class Amount;
class PasswordPrompt;
class String;
}  // namespace opentxs

namespace opentxs::factory
{
auto MintLucre(const api::Session& api) -> blind::Mint*;
auto MintLucre(
    const api::Session& api,
    const identifier::Server& notary,
    const identifier::UnitDefinition& unit) -> blind::Mint*;
auto MintLucre(
    const api::Session& api,
    const identifier::Server& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit) -> blind::Mint*;
auto Purse(const api::Session& api, const proto::Purse& serialized)
    -> blind::Purse*;
auto Purse(const api::Session& api, const ReadView& serialized)
    -> blind::Purse*;
auto Purse(
    const api::Session& api,
    const otx::context::Server&,
    const blind::CashType type,
    const blind::Mint& mint,
    const Amount& totalValue,
    const opentxs::PasswordPrompt& reason) -> blind::Purse*;
auto Purse(
    const api::Session& api,
    const identity::Nym& owner,
    const identifier::Server& server,
    const identity::Nym& serverNym,
    const blind::CashType type,
    const blind::Mint& mint,
    const Amount& totalValue,
    const opentxs::PasswordPrompt& reason) -> blind::Purse*;
auto Purse(
    const api::Session& api,
    const blind::Purse& request,
    const identity::Nym& requester,
    const opentxs::PasswordPrompt& reason) -> blind::Purse*;
auto Purse(
    const api::Session& api,
    const identity::Nym& owner,
    const identifier::Server& server,
    const identifier::UnitDefinition& unit,
    const blind::CashType type,
    const opentxs::PasswordPrompt& reason) -> blind::Purse*;
auto Token(const blind::Token& token, blind::Purse& purse) noexcept
    -> std::unique_ptr<blind::Token>;
auto Token(
    const api::Session& api,
    blind::Purse& purse,
    const proto::Token& serialized) noexcept(false)
    -> std::unique_ptr<blind::Token>;
auto Token(
    const api::Session& api,
    const identity::Nym& owner,
    const blind::Mint& mint,
    const blind::Token::Denomination value,
    blind::Purse& purse,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    -> std::unique_ptr<blind::Token>;
}  // namespace opentxs::factory

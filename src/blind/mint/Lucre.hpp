// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "blind/Mint.hpp"
#include "opentxs/core/Amount.hpp"

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

class PasswordPrompt;
class String;
}  // namespace opentxs

namespace opentxs::blind::mint::implementation
{
class Lucre final : public Mint
{
public:
    auto AddDenomination(
        const identity::Nym& theNotary,
        const Amount& denomination,
        const std::size_t keySize,
        const PasswordPrompt& reason) -> bool final;

    auto SignToken(
        const identity::Nym& notary,
        blind::Token& token,
        const PasswordPrompt& reason) -> bool final;
    auto VerifyToken(
        const identity::Nym& notary,
        const blind::Token& token,
        const PasswordPrompt& reason) -> bool final;

    Lucre(const api::Session& api);
    Lucre(
        const api::Session& api,
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit);
    Lucre(
        const api::Session& api,
        const identifier::Server& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit);

    ~Lucre() final = default;
};
}  // namespace opentxs::blind::mint::implementation

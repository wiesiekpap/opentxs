// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLIND_TOKEN_HPP
#define OPENTXS_BLIND_TOKEN_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>

#if OT_CASH
#include "opentxs/Pimpl.hpp"
#include "opentxs/blind/Types.hpp"

namespace opentxs
{
namespace blind
{
class Mint;
class Purse;
class Token;
}  // namespace blind

namespace identifier
{
class Server;
class UnitDefinition;
}  // namespace identifier

namespace proto
{
class Token;
}  // namespace proto

class PasswordPrompt;

using OTToken = Pimpl<blind::Token>;
}  // namespace opentxs

namespace opentxs
{
namespace blind
{
class OPENTXS_EXPORT Token
{
public:
    using Clock = std::chrono::system_clock;
    using Time = Clock::time_point;
    using Denomination = std::uint64_t;
    using MintSeries = std::uint64_t;

    virtual auto ID(const PasswordPrompt& reason) const -> std::string = 0;
    virtual auto IsSpent(const PasswordPrompt& reason) const -> bool = 0;
    virtual auto Notary() const -> const identifier::Server& = 0;
    virtual auto Owner() const noexcept -> Purse& = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(proto::Token& out) const noexcept
        -> bool = 0;
    virtual auto Series() const -> MintSeries = 0;
    virtual auto State() const -> blind::TokenState = 0;
    virtual auto Type() const -> blind::CashType = 0;
    virtual auto Unit() const -> const identifier::UnitDefinition& = 0;
    virtual auto ValidFrom() const -> Time = 0;
    virtual auto ValidTo() const -> Time = 0;
    virtual auto Value() const -> Denomination = 0;

    virtual auto ChangeOwner(
        Purse& oldOwner,
        Purse& newOwner,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto MarkSpent(const PasswordPrompt& reason) -> bool = 0;
    virtual auto Process(
        const identity::Nym& owner,
        const Mint& mint,
        const PasswordPrompt& reason) -> bool = 0;

    virtual ~Token() = default;

protected:
    Token() = default;

private:
    friend OTToken;

    virtual auto clone() const noexcept -> Token* = 0;

    Token(const Token&) = delete;
    Token(Token&&) = delete;
    auto operator=(const Token&) -> Token& = delete;
    auto operator=(Token&&) -> Token& = delete;
};
}  // namespace blind
}  // namespace opentxs
#endif  // OT_CASH
#endif

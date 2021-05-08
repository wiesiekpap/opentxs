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

    virtual std::string ID(const PasswordPrompt& reason) const = 0;
    virtual bool IsSpent(const PasswordPrompt& reason) const = 0;
    virtual const identifier::Server& Notary() const = 0;
    virtual Purse& Owner() const noexcept = 0;
    OPENTXS_NO_EXPORT virtual bool Serialize(
        proto::Token& out) const noexcept = 0;
    virtual MintSeries Series() const = 0;
    virtual blind::TokenState State() const = 0;
    virtual blind::CashType Type() const = 0;
    virtual const identifier::UnitDefinition& Unit() const = 0;
    virtual Time ValidFrom() const = 0;
    virtual Time ValidTo() const = 0;
    virtual Denomination Value() const = 0;

    virtual bool ChangeOwner(
        Purse& oldOwner,
        Purse& newOwner,
        const PasswordPrompt& reason) = 0;
    virtual bool MarkSpent(const PasswordPrompt& reason) = 0;
    virtual bool Process(
        const identity::Nym& owner,
        const Mint& mint,
        const PasswordPrompt& reason) = 0;

    virtual ~Token() = default;

protected:
    Token() = default;

private:
    friend OTToken;

    virtual Token* clone() const noexcept = 0;

    Token(const Token&) = delete;
    Token(Token&&) = delete;
    Token& operator=(const Token&) = delete;
    Token& operator=(Token&&) = delete;
};
}  // namespace blind
}  // namespace opentxs
#endif  // OT_CASH
#endif

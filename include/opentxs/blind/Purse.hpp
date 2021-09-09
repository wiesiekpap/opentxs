// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLIND_PURSE_HPP
#define OPENTXS_BLIND_PURSE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>

#if OT_CASH
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blind/Types.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs
{
namespace api
{
namespace server
{
class Manager;
}  // namespace server
}  // namespace api

namespace blind
{
class Mint;
class Purse;
class Token;
}  // namespace blind

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto

namespace identifier
{
class Server;
class UnitDefinition;
}  // namespace identifier

namespace proto
{
class Purse;
}  // namespace proto

class PasswordPrompt;

using OTPurse = Pimpl<blind::Purse>;
}  // namespace opentxs

namespace opentxs
{
namespace blind
{
class OPENTXS_EXPORT Purse
{
public:
    using Clock = std::chrono::system_clock;
    using Time = Clock::time_point;
    using iterator = opentxs::iterator::Bidirectional<Purse, Token>;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Purse, const Token>;

    virtual auto at(const std::size_t position) const -> const Token& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto cbegin() const noexcept -> const_iterator = 0;
    virtual auto cend() const noexcept -> const_iterator = 0;
    virtual auto EarliestValidTo() const -> Time = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto IsUnlocked() const -> bool = 0;
    virtual auto LatestValidFrom() const -> Time = 0;
    virtual auto Notary() const -> const identifier::Server& = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(proto::Purse& out) const noexcept
        -> bool = 0;
    virtual auto Serialize(AllocateOutput destination) const noexcept
        -> bool = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto State() const -> blind::PurseType = 0;
    virtual auto Type() const -> blind::CashType = 0;
    virtual auto Unit() const -> const identifier::UnitDefinition& = 0;
    virtual auto Unlock(const identity::Nym& nym, const PasswordPrompt& reason)
        const -> bool = 0;
    virtual auto Verify(const api::server::Manager& server) const -> bool = 0;
    virtual auto Value() const -> Amount = 0;

    virtual auto AddNym(const identity::Nym& nym, const PasswordPrompt& reason)
        -> bool = 0;
    virtual auto at(const std::size_t position) -> Token& = 0;
    virtual auto begin() noexcept -> iterator = 0;
    virtual auto end() noexcept -> iterator = 0;
    virtual auto PrimaryKey(PasswordPrompt& password) noexcept(false)
        -> crypto::key::Symmetric& = 0;
    virtual auto Pop() -> std::shared_ptr<Token> = 0;
    virtual auto Process(
        const identity::Nym& owner,
        const Mint& mint,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto Push(
        std::shared_ptr<Token> token,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto SecondaryKey(
        const identity::Nym& owner,
        PasswordPrompt& password) -> const crypto::key::Symmetric& = 0;

    virtual ~Purse() = default;

protected:
    Purse() noexcept = default;

private:
    friend OTPurse;

    virtual auto clone() const noexcept -> Purse* = 0;

    Purse(const Purse&) = delete;
    Purse(Purse&&) = delete;
    auto operator=(const Purse&) -> Purse& = delete;
    auto operator=(Purse&&) -> Purse& = delete;
};
}  // namespace blind
}  // namespace opentxs
#endif  // OT_CASH
#endif

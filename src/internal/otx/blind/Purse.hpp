// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/otx/blind/Types.hpp"

namespace opentxs
{
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

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace blind
{
class Mint;
}  // namespace blind
}  // namespace otx

namespace proto
{
class Purse;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::otx::blind::internal
{
class Purse
{
public:
    virtual auto Type() const -> blind::CashType = 0;
    virtual auto Unit() const -> const identifier::UnitDefinition& = 0;
    virtual auto Notary() const -> const identifier::Server& = 0;
    virtual auto Serialize(proto::Purse&) const noexcept -> bool = 0;

    virtual auto PrimaryKey(PasswordPrompt& password) noexcept(false)
        -> crypto::key::Symmetric& = 0;
    virtual auto Process(
        const identity::Nym&,
        const blind::Mint&,
        const PasswordPrompt&) -> bool = 0;
    virtual auto SecondaryKey(
        const identity::Nym& owner,
        PasswordPrompt& password) -> const crypto::key::Symmetric& = 0;

    virtual ~Purse() = default;
};
}  // namespace opentxs::otx::blind::internal

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace crypto
{
namespace key
{
class Asymmetric;
}  // namespace key
}  // namespace crypto

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace internal
{
class Contract;
}  // namespace internal

class Contract;
}  // namespace otx

class Identifier;
class PasswordPrompt;
class String;
}  // namespace opentxs

namespace opentxs::otx
{
class OPENTXS_EXPORT Contract
{
public:
    class Imp;

    auto GetIdentifier(Identifier& out) const noexcept -> void;
    auto GetIdentifier(String& out) const noexcept -> void;
    auto GetName(String& out) const noexcept -> void;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Contract&;
    auto VerifySignature(const identity::Nym& nym) const noexcept -> bool;
    auto VerifyWithKey(const crypto::key::Asymmetric& key) const noexcept
        -> bool;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Contract&;
    auto SignContract(
        const identity::Nym& nym,
        const PasswordPrompt& reason) noexcept -> bool;
    auto SignWithKey(
        const crypto::key::Asymmetric& key,
        const PasswordPrompt& reason) noexcept -> bool;

    virtual ~Contract();

protected:
    Contract(Imp* contract) noexcept(false);
    Contract(const Contract& rhs) noexcept;
    Contract(Contract&& rhs) noexcept;

private:
    Imp* contract_;

    Contract() = delete;
    auto operator=(const Contract&) -> Contract& = delete;
    auto operator=(Contract&&) -> Contract& = delete;
};
}  // namespace opentxs::otx

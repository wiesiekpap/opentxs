// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_NYMIDSOURCE_HPP
#define OPENTXS_CORE_NYMIDSOURCE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/identity/Types.hpp"

namespace opentxs
{
namespace identity
{
namespace credential
{
class Primary;
}  // namespace credential
}  // namespace identity

namespace proto
{
class NymIDSource;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace identity
{
class OPENTXS_EXPORT Source
{
public:
    virtual auto asString() const noexcept -> OTString = 0;
    virtual auto Description() const noexcept -> OTString = 0;
    virtual auto Type() const noexcept -> identity::SourceType = 0;
    virtual auto NymID() const noexcept -> OTNymID = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        proto::NymIDSource& serialized) const noexcept -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const noexcept -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Sign(
        const identity::credential::Primary& credential,
        proto::Signature& sig,
        const PasswordPrompt& reason) const noexcept -> bool = 0;

    virtual ~Source() = default;

protected:
    Source() = default;

private:
    Source(const Source&) = delete;
    Source(Source&&) = delete;
    auto operator=(const Source&) -> Source&;
    auto operator=(Source&&) -> Source&;
};
}  // namespace identity
}  // namespace opentxs
#endif

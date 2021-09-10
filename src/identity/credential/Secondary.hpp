// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "identity/credential/Base.hpp"
#include "identity/credential/Key.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace identity
{
namespace internal
{
struct Authority;
}  // namespace internal

class Source;
}  // namespace identity

namespace proto
{
class Credential;
}  // namespace proto

class Factory;
class NymParameters;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::identity::credential::implementation
{
class Secondary final : virtual public credential::internal::Secondary,
                        credential::implementation::Key
{
public:
    ~Secondary() override = default;

private:
    friend opentxs::Factory;

    auto serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const
        -> std::shared_ptr<Base::SerializedType> override;

    Secondary(
        const api::Core& api,
        const identity::internal::Authority& other,
        const identity::Source& source,
        const internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
    Secondary(
        const api::Core& api,
        const identity::internal::Authority& other,
        const identity::Source& source,
        const internal::Primary& master,
        const proto::Credential& serializedCred) noexcept(false);
    Secondary() = delete;
    Secondary(const Secondary&) = delete;
    Secondary(Secondary&&) = delete;
    auto operator=(const Secondary&) -> Secondary& = delete;
    auto operator=(Secondary&&) -> Secondary& = delete;
};
}  // namespace opentxs::identity::credential::implementation

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "Proto.hpp"
#include "core/contract/peer/PeerRequest.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/protobuf/PeerEnums.pb.h"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace identifier
{
class Nym;
class Server;
}  // namespace identifier

namespace proto
{
class PeerRequest;
}  // namespace proto

class Factory;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::contract::peer::request::implementation
{
class StoreSecret final : public request::StoreSecret,
                          public peer::implementation::Request
{
public:
    StoreSecret(
        const api::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const SecretType type,
        const std::string& primary,
        const std::string& secondary,
        const identifier::Server& serverID);
    StoreSecret(
        const api::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);

    ~StoreSecret() final = default;

    auto asStoreSecret() const noexcept -> const request::StoreSecret& final
    {
        return *this;
    }

private:
    friend opentxs::Factory;

    const SecretType secret_type_;
    const std::string primary_;
    const std::string secondary_;

    auto clone() const noexcept -> StoreSecret* final
    {
        return new StoreSecret(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;

    StoreSecret() = delete;
    StoreSecret(const StoreSecret&);
    StoreSecret(StoreSecret&&) = delete;
    auto operator=(const StoreSecret&) -> StoreSecret& = delete;
    auto operator=(StoreSecret&&) -> StoreSecret& = delete;
};
}  // namespace opentxs::contract::peer::request::implementation

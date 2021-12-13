// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/FactoryAPI.hpp"

#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

namespace google
{
namespace protobuf
{
class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Symmetric;
}  // namespace crypto
}  // namespace api

namespace identifier
{
class Nym;
class Server;
class Unit;
}  // namespace identifier

namespace proto
{
class Identifier;
}  // namespace proto

class Identifier;
}  // namespace opentxs

namespace opentxs::api::session::internal
{
class Factory : virtual public api::session::Factory,
                virtual public api::internal::Factory
{
public:
    using session::Factory::Armored;
    virtual auto Armored(const google::protobuf::MessageLite& input) const
        -> OTArmored = 0;
    virtual auto Armored(
        const google::protobuf::MessageLite& input,
        const std::string& header) const -> OTString = 0;
    virtual auto Asymmetric() const -> const api::crypto::Asymmetric& = 0;
    using session::Factory::Data;
    virtual auto Data(const google::protobuf::MessageLite& input) const
        -> OTData = 0;
    using session::Factory::Identifier;
    virtual auto Identifier(const google::protobuf::MessageLite& proto) const
        -> OTIdentifier = 0;
    virtual auto Identifier(const proto::Identifier& in) const noexcept
        -> OTIdentifier = 0;
    auto InternalSession() const noexcept -> const Factory& final
    {
        return *this;
    }
    using session::Factory::NymID;
    virtual auto NymID(const opentxs::Identifier& in) const noexcept
        -> OTNymID = 0;
    virtual auto NymID(const proto::Identifier& in) const noexcept
        -> OTNymID = 0;
    using session::Factory::ServerID;
    virtual auto ServerID(const opentxs::Identifier& in) const noexcept
        -> OTServerID = 0;
    virtual auto ServerID(const proto::Identifier& in) const noexcept
        -> OTServerID = 0;
    virtual auto ServerID(const google::protobuf::MessageLite& proto) const
        -> OTIdentifier = 0;
    virtual auto Symmetric() const -> const api::crypto::Symmetric& = 0;
    using session::Factory::UnitID;
    virtual auto UnitID(const opentxs::Identifier& in) const noexcept
        -> OTUnitID = 0;
    virtual auto UnitID(const proto::Identifier& in) const noexcept
        -> OTUnitID = 0;
    virtual auto UnitID(const google::protobuf::MessageLite& proto) const
        -> OTIdentifier = 0;

    auto InternalSession() noexcept -> Factory& final { return *this; }

    ~Factory() override = default;
};
}  // namespace opentxs::api::session::internal

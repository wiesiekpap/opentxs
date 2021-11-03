// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/core/AddressType.hpp"

#pragma once

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/NymFile.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/ContractEnums.pb.h"
#include "opentxs/protobuf/PeerEnums.pb.h"
#include "opentxs/util/Bytes.hpp"
#include "util/Blank.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

class Identifier;
class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace opentxs
{
template <typename T>
struct make_blank;

template <>
struct make_blank<OTData> {
    static auto value(const api::Session&) -> OTData { return Data::Factory(); }
};
template <>
struct make_blank<OTIdentifier> {
    static auto value(const api::Session&) -> OTIdentifier
    {
        return Identifier::Factory();
    }
};
}  // namespace opentxs

namespace opentxs::internal
{
struct NymFile : virtual public opentxs::NymFile {
    virtual auto LoadSignedNymFile(const PasswordPrompt& reason) -> bool = 0;
    virtual auto SaveSignedNymFile(const PasswordPrompt& reason) -> bool = 0;
};
}  // namespace opentxs::internal

namespace opentxs::blockchain
{
auto AccountName(const Type chain) noexcept -> std::string;
auto Chain(const api::Session& api, const identifier::Nym& id) noexcept -> Type;
auto Chain(const api::Session& api, const identifier::Server& id) noexcept
    -> Type;
auto Chain(
    const api::Session& api,
    const identifier::UnitDefinition& id) noexcept -> Type;
auto IssuerID(const api::Session& api, const Type chain) noexcept
    -> const identifier::Nym&;
auto NotaryID(const api::Session& api, const Type chain) noexcept
    -> const identifier::Server&;
auto UnitID(const api::Session& api, const Type chain) noexcept
    -> const identifier::UnitDefinition&;
}  // namespace opentxs::blockchain

namespace opentxs::factory
{
auto Secret(const std::size_t bytes) noexcept
    -> std::unique_ptr<opentxs::Secret>;
auto Secret(const ReadView bytes, const bool mode) noexcept
    -> std::unique_ptr<opentxs::Secret>;
}  // namespace opentxs::factory

namespace opentxs
{
auto translate(const core::AddressType in) noexcept -> proto::AddressType;
auto translate(const proto::AddressType in) noexcept -> core::AddressType;
}  // namespace opentxs

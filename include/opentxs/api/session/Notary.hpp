// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <string>

#include "opentxs/api/session/Session.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Notary;
}  // namespace internal
}  // namespace session
}  // namespace api

namespace blind
{
class Mint;
}  // namespace blind

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace server
{
class Server;
}  // namespace server

class Options;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace session
{
class OPENTXS_EXPORT Notary : virtual public api::Session
{
public:
    static auto DefaultMintKeyBytes() noexcept -> std::size_t;

    /** Drop a specified number of incoming requests for testing purposes */
    virtual auto DropIncoming(const int count) const -> void = 0;
    /** Drop a specified number of outgoing replies for testing purposes */
    virtual auto DropOutgoing(const int count) const -> void = 0;
    virtual auto GetAdminNym() const -> std::string = 0;
    virtual auto GetAdminPassword() const -> std::string = 0;
#if OT_CASH
    virtual auto GetPrivateMint(
        const identifier::UnitDefinition& unitid,
        std::uint32_t series) const -> std::shared_ptr<blind::Mint> = 0;
    virtual auto GetPublicMint(const identifier::UnitDefinition& unitID) const
        -> std::shared_ptr<const blind::Mint> = 0;
#endif  // OT_CASH
    virtual auto GetUserName() const -> std::string = 0;
    virtual auto GetUserTerms() const -> std::string = 0;
    virtual auto ID() const -> const identifier::Server& = 0;
    OPENTXS_NO_EXPORT virtual auto InternalNotary() const noexcept
        -> const session::internal::Notary& = 0;
    virtual auto NymID() const -> const identifier::Nym& = 0;
    virtual auto ScanMints() const -> void = 0;
    virtual auto Server() const -> opentxs::server::Server& = 0;
    virtual auto SetMintKeySize(const std::size_t size) const -> void = 0;
    virtual auto UpdateMint(const identifier::UnitDefinition& unitID) const
        -> void = 0;

    OPENTXS_NO_EXPORT virtual auto InternalNotary() noexcept
        -> session::internal::Notary& = 0;

    OPENTXS_NO_EXPORT ~Notary() override = default;

protected:
    Notary() = default;

private:
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    auto operator=(const Notary&) -> Notary& = delete;
    auto operator=(Notary&&) -> Notary& = delete;
};
}  // namespace session
}  // namespace api
}  // namespace opentxs

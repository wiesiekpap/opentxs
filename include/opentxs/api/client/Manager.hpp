// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_MANAGER_HPP
#define OPENTXS_API_CLIENT_MANAGER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/api/Core.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal

class Activity;
class Blockchain;
class Contacts;
class OTX;
class Pair;
class ServerAction;
class UI;
class Workflow;
}  // namespace client

namespace network
{
class ZMQ;
}  // namespace network
}  // namespace api

class OT_API;
class OTAPI_Exec;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace client
{
class OPENTXS_EXPORT Manager : virtual public api::Core
{
public:
    virtual auto Activity() const -> const api::client::Activity& = 0;
    virtual auto Blockchain() const -> const api::client::Blockchain& = 0;
    virtual auto Contacts() const -> const api::client::Contacts& = 0;
    virtual auto Exec(const std::string& wallet = "") const
        -> const OTAPI_Exec& = 0;
    OPENTXS_NO_EXPORT virtual auto InternalClient() const noexcept
        -> internal::Manager& = 0;
    virtual auto Lock(
        const identifier::Nym& nymID,
        const identifier::Server& serverID) const -> std::recursive_mutex& = 0;
    virtual auto OTAPI(const std::string& wallet = "") const
        -> const OT_API& = 0;
    virtual auto OTX() const -> const client::OTX& = 0;
    virtual auto Pair() const -> const client::Pair& = 0;
    virtual auto ServerAction() const -> const client::ServerAction& = 0;
    virtual auto UI() const -> const api::client::UI& = 0;
    virtual auto Workflow() const -> const client::Workflow& = 0;
    virtual auto ZMQ() const -> const network::ZMQ& = 0;

    OPENTXS_NO_EXPORT ~Manager() override = default;

protected:
    Manager() = default;

private:
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    auto operator=(const Manager&) -> Manager& = delete;
    auto operator=(Manager&&) -> Manager& = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif

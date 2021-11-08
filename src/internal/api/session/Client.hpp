// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/session/Session.hpp"
#include "opentxs/api/session/Client.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct UI;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace identifier
{
class Nym;
class Server;
}  // namespace identifier

class OT_API;
class OTAPI_Exec;
}  // namespace opentxs

namespace opentxs::api::session::internal
{
class Client : virtual public session::Client, virtual public Session
{
public:
    virtual auto Exec(const std::string& wallet = "") const
        -> const OTAPI_Exec& = 0;
    auto InternalClient() const noexcept -> const internal::Client& final
    {
        return *this;
    }
    virtual auto InternalUI() const noexcept -> const client::internal::UI& = 0;
    using Session::Lock;
    virtual auto Lock(
        const identifier::Nym& nymID,
        const identifier::Server& serverID) const -> std::recursive_mutex& = 0;
    virtual auto OTAPI(const std::string& wallet = "") const
        -> const OT_API& = 0;

    virtual auto Init() -> void = 0;
    auto InternalClient() noexcept -> internal::Client& final { return *this; }

    ~Client() override = default;
};
}  // namespace opentxs::api::session::internal

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/api/session/Session.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Activity;
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

namespace session
{
namespace internal
{
class Client;
}  // namespace internal
}  // namespace session
}  // namespace api
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace session
{
class OPENTXS_EXPORT Client : virtual public api::Session
{
public:
    virtual auto Activity() const -> const api::client::Activity& = 0;
    virtual auto Contacts() const -> const api::client::Contacts& = 0;
    OPENTXS_NO_EXPORT virtual auto InternalClient() const noexcept
        -> const internal::Client& = 0;
    virtual auto OTX() const -> const api::client::OTX& = 0;
    virtual auto Pair() const -> const api::client::Pair& = 0;
    virtual auto ServerAction() const -> const api::client::ServerAction& = 0;
    virtual auto UI() const -> const api::client::UI& = 0;
    virtual auto Workflow() const -> const api::client::Workflow& = 0;
    virtual auto ZMQ() const -> const network::ZMQ& = 0;

    OPENTXS_NO_EXPORT virtual auto InternalClient() noexcept
        -> internal::Client& = 0;

    OPENTXS_NO_EXPORT ~Client() override = default;

protected:
    Client() = default;

private:
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    auto operator=(const Client&) -> Client& = delete;
    auto operator=(Client&&) -> Client& = delete;
};
}  // namespace session
}  // namespace api
}  // namespace opentxs

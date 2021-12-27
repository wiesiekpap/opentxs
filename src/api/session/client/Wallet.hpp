// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

#include "api/session/Wallet.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
class Wallet;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
class Server;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
namespace internal
{
struct Base;
}  // namespace internal

class Base;
class Server;
}  // namespace context

class Base;
class Server;
}  // namespace otx

namespace proto
{
class Context;
}  // namespace proto

class Context;
class Identifier;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::session::client
{
class Wallet final : public session::imp::Wallet
{
public:
    auto Context(
        const identifier::Server& notaryID,
        const identifier::Nym& clientNymID) const
        -> std::shared_ptr<const otx::context::Base> final;
    auto mutable_Context(
        const identifier::Server& notaryID,
        const identifier::Nym& clientNymID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Base> final;
    auto mutable_ServerContext(
        const identifier::Nym& localNymID,
        const Identifier& remoteID,
        const PasswordPrompt& reason) const
        -> Editor<otx::context::Server> final;
    auto ServerContext(
        const identifier::Nym& localNymID,
        const Identifier& remoteID) const
        -> std::shared_ptr<const otx::context::Server> final;

    Wallet(const api::session::Client& parent);

    ~Wallet() final = default;

private:
    using ot_super = session::imp::Wallet;

    const api::session::Client& client_;
    OTZMQPublishSocket request_sent_;
    OTZMQPublishSocket reply_received_;

    void instantiate_server_context(
        const proto::Context& serialized,
        const Nym_p& localNym,
        const Nym_p& remoteNym,
        std::shared_ptr<otx::context::internal::Base>& output) const final;
    void nym_to_contact(const identity::Nym& nym, const std::string& name)
        const noexcept final;
    auto signer_nym(const identifier::Nym& id) const -> Nym_p final;

    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::api::session::client

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

#include "api/Wallet.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client

class Wallet;
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

namespace internal
{
struct Context;
}  // namespace internal

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

namespace opentxs::api::client::implementation
{
class Wallet final : public api::implementation::Wallet
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

    Wallet(const api::client::Manager& client);

    ~Wallet() final = default;

private:
    using ot_super = api::implementation::Wallet;

    const api::client::Manager& client_;
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
}  // namespace opentxs::api::client::implementation

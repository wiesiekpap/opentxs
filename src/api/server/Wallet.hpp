// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "api/Wallet.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"

namespace opentxs
{
namespace api
{
namespace server
{
class Manager;
}  // namespace server

class Wallet;
}  // namespace api

namespace identifier
{
class Nym;
class Server;
}  // namespace identifier

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
class Client;
}  // namespace context

class Base;
class Client;
}  // namespace otx

namespace proto
{
class Context;
}  // namespace proto

class Factory;
class Identifier;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::server::implementation
{
class Wallet final : public api::implementation::Wallet
{
public:
    auto ClientContext(const identifier::Nym& remoteNymID) const
        -> std::shared_ptr<const otx::context::Client> final;
    auto Context(
        const identifier::Server& notaryID,
        const identifier::Nym& clientNymID) const
        -> std::shared_ptr<const otx::context::Base> final;
    auto mutable_ClientContext(
        const identifier::Nym& remoteNymID,
        const PasswordPrompt& reason) const
        -> Editor<otx::context::Client> final;
    auto mutable_Context(
        const identifier::Server& notaryID,
        const identifier::Nym& clientNymID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Base> final;

    ~Wallet() final = default;

private:
    friend opentxs::Factory;

    using ot_super = api::implementation::Wallet;

    const api::server::Manager& server_;

    void instantiate_client_context(
        const proto::Context& serialized,
        const Nym_p& localNym,
        const Nym_p& remoteNym,
        std::shared_ptr<otx::context::internal::Base>& output) const final;
    auto load_legacy_account(
        const Identifier& accountID,
        const eLock& lock,
        AccountLock& row) const -> bool final;
    auto signer_nym(const identifier::Nym& id) const -> Nym_p final;

    Wallet(const api::server::Manager& server);
    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::api::server::implementation

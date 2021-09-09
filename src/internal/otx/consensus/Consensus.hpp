// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/otx/consensus/Base.hpp"
#include "opentxs/otx/consensus/Client.hpp"
#include "opentxs/otx/consensus/Server.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::otx::context::internal
{
struct Base : virtual public opentxs::otx::context::Base {
    virtual auto GetContract(const Lock& lock) const -> proto::Context = 0;
    virtual auto ValidateContext(const Lock& lock) const -> bool = 0;

    virtual auto GetLock() -> std::mutex& = 0;
    virtual auto UpdateSignature(const Lock& lock, const PasswordPrompt& reason)
        -> bool = 0;

#ifdef _MSC_VER
    Base() {}
#endif  // _MSC_VER
    ~Base() override = default;
};
struct Client : virtual public opentxs::otx::context::Client,
                virtual public otx::context::internal::Base {

#ifdef _MSC_VER
    Client() {}
#endif  // _MSC_VER
    ~Client() override = default;
};
struct Server : virtual public opentxs::otx::context::Server,
                virtual public otx::context::internal::Base {

#ifdef _MSC_VER
    Server() {}
#endif  // _MSC_VER
    ~Server() override = default;
};
}  // namespace opentxs::otx::context::internal

namespace opentxs::factory
{
auto ClientContext(
    const api::Core& api,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Server& server) -> otx::context::internal::Client*;
auto ClientContext(
    const api::Core& api,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Server& server) -> otx::context::internal::Client*;
auto ManagedNumber(const TransactionNumber number, otx::context::Server&)
    -> otx::context::ManagedNumber*;
auto ServerContext(
    const api::client::Manager& api,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Server& server,
    network::ServerConnection& connection) -> otx::context::internal::Server*;
auto ServerContext(
    const api::client::Manager& api,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    network::ServerConnection& connection) -> otx::context::internal::Server*;
}  // namespace opentxs::factory

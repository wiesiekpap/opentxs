// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

#include "opentxs/Types.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Contacts;
struct Pair;
struct UI;
}  // namespace internal

class Activity;
class Contacts;
class Issuer;
class OTX;
class ServerAction;
class Workflow;
}  // namespace client

namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace internal
{
class Context;
}  // namespace internal

namespace session
{
class Client;
class Wallet;
}  // namespace session

class Crypto;
class Legacy;
class Session;
class Settings;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class Issuer;
}  // namespace proto

class Flag;
class OTClient;
class Options;
}  // namespace opentxs

namespace opentxs::factory
{
auto ActivityAPI(
    const api::Session& api,
    const api::client::Contacts& contact) noexcept
    -> std::unique_ptr<api::client::Activity>;
auto ContactAPI(const api::session::Client& api) noexcept
    -> std::unique_ptr<api::client::internal::Contacts>;
auto Issuer(
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const proto::Issuer& serialized) -> api::client::Issuer*;
auto Issuer(
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID) -> api::client::Issuer*;
auto OTX(
    const Flag& running,
    const api::session::Client& api,
    OTClient& otclient,
    const ContextLockCallback& lockCallback) -> api::client::OTX*;
auto PairAPI(const Flag& running, const api::session::Client& client)
    -> api::client::internal::Pair*;
auto ServerAction(
    const api::session::Client& api,
    const ContextLockCallback& lockCallback) -> api::client::ServerAction*;
auto UI(
    const api::session::Client& api,
    const api::crypto::Blockchain& blockchain,
    const Flag& running) noexcept -> std::unique_ptr<api::client::internal::UI>;
auto Workflow(
    const api::Session& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contact) -> api::client::Workflow*;
}  // namespace opentxs::factory

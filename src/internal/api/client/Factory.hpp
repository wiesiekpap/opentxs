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
struct Blockchain;
struct Contacts;
struct Pair;
struct UI;
}  // namespace internal

class Activity;
class Contacts;
class Issuer;
class Manager;
class OTX;
class ServerAction;
class Workflow;
}  // namespace client

namespace internal
{
struct Context;
struct Factory;
}  // namespace internal

class Core;
class Crypto;
class Legacy;
class Settings;
class Wallet;
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
    const api::Core& api,
    const api::client::Contacts& contact) noexcept
    -> std::unique_ptr<api::client::Activity>;
auto BlockchainAPI(
    const api::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const Options& args) noexcept
    -> std::shared_ptr<api::client::internal::Blockchain>;
auto ClientManager(
    const api::internal::Context& parent,
    Flag& running,
    Options&& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance) noexcept -> std::unique_ptr<api::client::Manager>;
auto ContactAPI(const api::client::Manager& api) noexcept
    -> std::unique_ptr<api::client::internal::Contacts>;
auto FactoryAPIClient(const api::client::Manager& api)
    -> api::internal::Factory*;
auto Issuer(
    const api::Wallet& wallet,
    const identifier::Nym& nymID,
    const proto::Issuer& serialized) -> api::client::Issuer*;
auto Issuer(
    const api::Wallet& wallet,
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID) -> api::client::Issuer*;
auto OTX(
    const Flag& running,
    const api::client::Manager& api,
    OTClient& otclient,
    const ContextLockCallback& lockCallback) -> api::client::OTX*;
auto PairAPI(const Flag& running, const api::client::Manager& client)
    -> api::client::internal::Pair*;
auto ServerAction(
    const api::client::Manager& api,
    const ContextLockCallback& lockCallback) -> api::client::ServerAction*;
auto UI(
    const api::client::Manager& api,
    const api::client::internal::Blockchain& blockchain,
    const Flag& running) noexcept -> std::unique_ptr<api::client::internal::UI>;
auto Wallet(const api::client::Manager& client) -> api::Wallet*;
auto Workflow(
    const api::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contact) -> api::client::Workflow*;
}  // namespace opentxs::factory

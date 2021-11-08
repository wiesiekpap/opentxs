// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network

namespace session
{
class Client;
class Crypto;
class Endpoints;
class Factory;
class Notary;
class Storage;
class Wallet;
}  // namespace session

class Context;
class Crypto;
class Session;
class Settings;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace storage
{
class Config;
}  // namespace storage

class Flag;
class Options;
}  // namespace opentxs

namespace opentxs::factory
{
auto ClientSession(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance) noexcept -> std::unique_ptr<api::session::Client>;
auto EndpointsAPI(
    const network::zeromq::Context& zmq,
    const int instance) noexcept -> std::unique_ptr<api::session::Endpoints>;
auto NotarySession(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance) -> std::unique_ptr<api::session::Notary>;
auto SessionCryptoAPI(
    api::Crypto& parent,
    const api::Session& session,
    const api::session::Factory& factory,
    const api::session::Storage& storage) noexcept
    -> std::unique_ptr<api::session::Crypto>;
auto SessionFactoryAPI(const api::session::Client& parent) noexcept
    -> std::unique_ptr<api::session::Factory>;
auto SessionFactoryAPI(const api::session::Notary& parent) noexcept
    -> std::unique_ptr<api::session::Factory>;
auto StorageAPI(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const Flag& running,
    const opentxs::storage::Config& config) noexcept
    -> std::unique_ptr<api::session::Storage>;
auto WalletAPI(const api::session::Client& parent) noexcept
    -> std::unique_ptr<api::session::Wallet>;
auto WalletAPI(const api::session::Notary& parent) noexcept
    -> std::unique_ptr<api::session::Wallet>;
}  // namespace opentxs::factory

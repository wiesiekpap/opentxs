// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/api/Context.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
namespace internal
{
struct Asymmetric;
}  // namespace internal

class Symmetric;
}  // namespace crypto

namespace internal
{
struct Context;
struct Log;
}  // namespace internal

namespace storage
{
class Storage;
}  // namespace storage

class Core;
class Crypto;
class Endpoints;
class Factory;
class HDSeed;
class Legacy;
class Primitives;
class Settings;
}  // namespace api

namespace crypto
{
class Bip32;
class Bip39;
}  // namespace crypto

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

class Flag;
class Options;
class OTCaller;
class String;
}  // namespace opentxs

namespace opentxs::factory
{
auto Context(
    Flag& running,
    const Options& args,
    OTCaller* externalPasswordCallback = nullptr) noexcept
    -> std::unique_ptr<api::internal::Context>;
auto Endpoints(const network::zeromq::Context& zmq, const int instance) noexcept
    -> std::unique_ptr<api::Endpoints>;
auto HDSeed(
    const api::Core& api,
    const api::Factory& factory,
    const api::crypto::internal::Asymmetric& asymmetric,
    const api::crypto::Symmetric& symmetric,
    const api::storage::Storage& storage,
    const crypto::Bip32& bip32,
    const crypto::Bip39& bip39) noexcept -> std::unique_ptr<api::HDSeed>;
auto Legacy(const std::string& home) noexcept -> std::unique_ptr<api::Legacy>;
auto Log(
    const network::zeromq::Context& zmq,
    const std::string& endpoint) noexcept
    -> std::unique_ptr<api::internal::Log>;
auto Primitives(const api::Crypto& crypto) noexcept
    -> std::unique_ptr<api::Primitives>;
auto Settings(const api::Legacy& legacy, const String& path) noexcept
    -> std::unique_ptr<api::Settings>;
}  // namespace opentxs::factory

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
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

namespace storage
{
namespace internal
{
class Storage;
}  // namespace internal

class Config;
}  // namespace storage

class Crypto;
}  // namespace api

class Flag;
}  // namespace opentxs

namespace opentxs::factory
{
auto StorageInterface(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const Flag& running,
    const opentxs::storage::Config& config) noexcept
    -> std::unique_ptr<api::storage::internal::Storage>;
}  // namespace opentxs::factory

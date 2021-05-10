// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CORE_HPP
#define OPENTXS_API_CORE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>

#include "opentxs/api/Editor.hpp"
#include "opentxs/api/Periodic.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Symmetric;
}  // namespace crypto

namespace network
{
class Asio;
class Dht;
}  // namespace network

namespace storage
{
class Storage;
}  // namespace storage

class Crypto;
class Endpoints;
class Factory;
class HDSeed;
class Settings;
class ThreadPool;
class Wallet;
}  // namespace api
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class OPENTXS_EXPORT Core : virtual public Periodic
{
public:
    virtual const network::Asio& Asio() const noexcept = 0;
    virtual const crypto::Asymmetric& Asymmetric() const = 0;
    virtual const api::Settings& Config() const = 0;
    virtual const api::Crypto& Crypto() const = 0;
    virtual const std::string& DataFolder() const = 0;
    virtual const api::Endpoints& Endpoints() const = 0;
    virtual const network::Dht& DHT() const = 0;
    virtual const api::Factory& Factory() const = 0;
    virtual int Instance() const = 0;
    virtual const api::HDSeed& Seeds() const = 0;
    virtual void SetMasterKeyTimeout(
        const std::chrono::seconds& timeout) const = 0;
    OPENTXS_NO_EXPORT virtual const storage::Storage& Storage() const = 0;
    virtual const crypto::Symmetric& Symmetric() const = 0;
    virtual const api::ThreadPool& ThreadPool() const noexcept = 0;
    virtual const api::Wallet& Wallet() const = 0;
    virtual const opentxs::network::zeromq::Context& ZeroMQ() const = 0;

    ~Core() override = default;

protected:
    Core() = default;

private:
    Core(const Core&) = delete;
    Core(Core&&) = delete;
    Core& operator=(const Core&) = delete;
    Core& operator=(Core&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif

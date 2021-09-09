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

namespace internal
{
struct Core;
}  // namespace internal

namespace network
{
class Network;
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
class Wallet;
}  // namespace api

class Options;
}  // namespace opentxs

class QObject;

namespace opentxs
{
namespace api
{
class OPENTXS_EXPORT Core : virtual public Periodic
{
public:
    virtual auto Asymmetric() const noexcept -> const crypto::Asymmetric& = 0;
    virtual auto Config() const noexcept -> const api::Settings& = 0;
    virtual auto Crypto() const noexcept -> const api::Crypto& = 0;
    virtual auto DataFolder() const noexcept -> const std::string& = 0;
    virtual auto Endpoints() const noexcept -> const api::Endpoints& = 0;
    virtual auto Factory() const noexcept -> const api::Factory& = 0;
    virtual auto GetOptions() const noexcept -> const Options& = 0;
    virtual auto Instance() const noexcept -> int = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> internal::Core& = 0;
    virtual auto Network() const noexcept -> const network::Network& = 0;
    virtual auto QtRootObject() const noexcept -> QObject* = 0;
    virtual auto Seeds() const noexcept -> const api::HDSeed& = 0;
    virtual auto SetMasterKeyTimeout(
        const std::chrono::seconds& timeout) const noexcept -> void = 0;
    OPENTXS_NO_EXPORT virtual auto Storage() const noexcept
        -> const storage::Storage& = 0;
    virtual auto Symmetric() const noexcept
        -> const api::crypto::Symmetric& = 0;
    virtual auto Wallet() const noexcept -> const api::Wallet& = 0;

    ~Core() override = default;

protected:
    Core() = default;

private:
    Core(const Core&) = delete;
    Core(Core&&) = delete;
    auto operator=(const Core&) -> Core& = delete;
    auto operator=(Core&&) -> Core& = delete;
};
}  // namespace api
}  // namespace opentxs
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "api/Core.hpp"
#include "internal/api/server/Server.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "server/Server.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Context;
}  // namespace internal

class Crypto;
class Settings;
}  // namespace api

namespace blind
{
class Mint;
}  // namespace blind

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace server
{
class MessageProcessor;
}  // namespace server

class Factory;
class Flag;
class Options;
}  // namespace opentxs

namespace opentxs::api::server::implementation
{
class Manager final : internal::Manager, api::implementation::Core
{
public:
    auto DropIncoming(const int count) const -> void final;
    auto DropOutgoing(const int count) const -> void final;
    auto GetAdminNym() const -> std::string final;
    auto GetAdminPassword() const -> std::string final;
#if OT_CASH
    auto GetPrivateMint(
        const identifier::UnitDefinition& unitID,
        std::uint32_t series) const -> std::shared_ptr<blind::Mint> final;
    auto GetPublicMint(const identifier::UnitDefinition& unitID) const
        -> std::shared_ptr<const blind::Mint> final;
#endif  // OT_CASH
    auto GetUserName() const -> std::string final;
    auto GetUserTerms() const -> std::string final;
    auto ID() const -> const identifier::Server& final;
    auto InternalServer() const noexcept -> internal::Manager& final
    {
        return const_cast<Manager&>(*this);
    }
    auto MakeInprocEndpoint() const -> std::string final;
    auto NymID() const -> const identifier::Nym& final;
    auto ScanMints() const -> void final;
    auto Server() const -> opentxs::server::Server& final { return server_; }
    auto SetMintKeySize(const std::size_t size) const -> void final
    {
#if OT_CASH
        mint_key_size_.store(size);
#endif  // OT_CASH
    }
    auto UpdateMint(const identifier::UnitDefinition& unitID) const
        -> void final;

    ~Manager() final;

private:
    friend opentxs::Factory;

#if OT_CASH
    using MintSeries = std::map<std::string, std::shared_ptr<blind::Mint>>;
#endif  // OT_CASH

    const OTPasswordPrompt reason_;
    std::unique_ptr<opentxs::server::Server> server_p_;
    opentxs::server::Server& server_;
    std::unique_ptr<opentxs::server::MessageProcessor> message_processor_p_;
    opentxs::server::MessageProcessor& message_processor_;
#if OT_CASH
    std::thread mint_thread_;
    mutable std::mutex mint_lock_;
    mutable std::mutex mint_update_lock_;
    mutable std::mutex mint_scan_lock_;
    mutable std::map<std::string, MintSeries> mints_;
    mutable std::deque<std::string> mints_to_check_;
    mutable std::atomic<std::size_t> mint_key_size_;
#endif  // OT_CASH

#if OT_CASH
    void generate_mint(
        const std::string& serverID,
        const std::string& unitID,
        const std::uint32_t series) const;
    auto last_generated_series(
        const std::string& serverID,
        const std::string& unitID) const -> std::int32_t;
    auto load_private_mint(
        const opentxs::Lock& lock,
        const std::string& unitID,
        const std::string seriesID) const -> std::shared_ptr<blind::Mint>;
    auto load_public_mint(
        const opentxs::Lock& lock,
        const std::string& unitID,
        const std::string seriesID) const -> std::shared_ptr<blind::Mint>;
    void mint() const;
#endif  // OT_CASH
    auto verify_lock(const opentxs::Lock& lock, const std::mutex& mutex) const
        -> bool;
#if OT_CASH
    auto verify_mint(
        const opentxs::Lock& lock,
        const std::string& unitID,
        const std::string seriesID,
        std::shared_ptr<blind::Mint>& mint) const
        -> std::shared_ptr<blind::Mint>;
    auto verify_mint_directory(const std::string& serverID) const -> bool;
#endif  // OT_CASH

    void Cleanup();
    void Init();
    void Start() final;

    Manager(
        const api::internal::Context& parent,
        Flag& running,
        Options&& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    Manager() = delete;
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    auto operator=(const Manager&) -> Manager& = delete;
    auto operator=(Manager&&) -> Manager& = delete;
};
}  // namespace opentxs::api::server::implementation

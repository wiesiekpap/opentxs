// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/interprocess/sync/file_lock.hpp>
#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "api/Periodic.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Legacy.hpp"
#include "opentxs/api/Primitives.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/ThreadPool.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/ZAP.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/protobuf/RPCResponse.pb.h"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/response/Base.hpp"

namespace opentxs
{
namespace proto
{
class RPCCommand;
}  // namespace proto

namespace rpc
{
namespace internal
{
struct RPC;
}  // namespace internal

namespace request
{
class Base;
}  // namespace request

namespace response
{
class Base;
}  // namespace response
}  // namespace rpc

class Flag;
class OTCallback;
class OTCaller;
class Signals;
}  // namespace opentxs

namespace opentxs::api::implementation
{
class Context final : public api::internal::Context, Lockable, Periodic
{
public:
    auto Asio() const noexcept -> const network::Asio& final { return *asio_; }
    auto Client(const int instance) const
        -> const api::client::internal::Manager& final;
    auto Clients() const -> std::size_t final { return client_.size(); }
    auto Config(const std::string& path) const -> const api::Settings& final;
    auto Crypto() const -> const api::Crypto& final;
    auto Factory() const -> const api::Primitives& final;
    void HandleSignals(ShutdownCallback* shutdown) const final;
    auto Legacy() const noexcept -> const api::Legacy& final
    {
        return *legacy_;
    }
    auto ProfileId() const -> std::string final;
    auto RPC(const rpc::request::Base& command) const noexcept
        -> std::unique_ptr<rpc::response::Base> final;
    auto RPC(const ReadView command, const AllocateOutput response)
        const noexcept -> bool final;
    auto Server(const int instance) const -> const api::server::Manager& final;
    auto Servers() const -> std::size_t final { return server_.size(); }
    auto StartClient(const ArgList& args, const int instance) const
        -> const api::client::internal::Manager& final;
#if OT_CRYPTO_WITH_BIP32
    auto StartClient(
        const ArgList& args,
        const int instance,
        const std::string& recoverWords,
        const std::string& recoverPassphrase) const
        -> const api::client::internal::Manager& final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto StartServer(const ArgList& args, const int instance, const bool inproc)
        const -> const api::server::Manager& final;
    auto ThreadPool() const noexcept -> const api::ThreadPool& final
    {
        return *thread_pool_;
    }
    auto ZAP() const -> const api::network::ZAP& final;
    auto ZMQ() const -> const opentxs::network::zeromq::Context& final
    {
        return zmq_context_.get();
    }

    auto GetPasswordCaller() const -> OTCaller& final;

    Context(
        Flag& running,
        const ArgList& args,
        OTCaller* externalPasswordCallback = nullptr);

    ~Context() final;

private:
    using ConfigMap = std::map<std::string, std::unique_ptr<api::Settings>>;

    const std::string home_;
    mutable std::mutex config_lock_;
    mutable std::mutex task_list_lock_;
    mutable std::mutex signal_handler_lock_;
    mutable ConfigMap config_;
    OTZMQContext zmq_context_;
    mutable std::unique_ptr<Signals> signal_handler_;
    std::unique_ptr<api::internal::Log> log_;
    std::unique_ptr<network::Asio> asio_;
    std::unique_ptr<api::internal::ThreadPool> thread_pool_;
    std::unique_ptr<api::internal::Crypto> crypto_;
    std::unique_ptr<api::Primitives> factory_;
    std::unique_ptr<api::Legacy> legacy_;
    std::unique_ptr<api::network::ZAP> zap_;
    const ArgList args_;
    std::string profile_id_;
    mutable ShutdownCallback* shutdown_callback_;
    std::unique_ptr<OTCallback> null_callback_;
    std::unique_ptr<OTCaller> default_external_password_callback_;
    OTCaller* external_password_callback_;
    mutable boost::interprocess::file_lock file_lock_;
    mutable std::vector<std::unique_ptr<api::server::Manager>> server_;
    mutable std::vector<std::unique_ptr<api::client::internal::Manager>>
        client_;
    std::unique_ptr<rpc::internal::RPC> rpc_;

    static auto client_instance(const int count) -> int;
    static auto server_instance(const int count) -> int;
    static auto get_arg(const ArgList& args, const std::string& argName)
        -> std::string;

    void init_pid() const;
    auto merge_arglist(const ArgList& args) const -> const ArgList;
    void start_client(const Lock& lock, const ArgList& args) const;
    void start_server(const Lock& lock, const ArgList& args) const;

    void Init_Asio();
    void Init_Crypto();
    void Init_Factory();
    void Init_Log(const std::int32_t argLevel);
#ifndef _WIN32
    void Init_Rlimit() noexcept;
#endif  // _WIN32
    void Init_Profile();
    void Init_Zap();
    void Init() final;
    void setup_default_external_password_callback();
    void shutdown() final;

    Context() = delete;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;
};
}  // namespace opentxs::api::implementation

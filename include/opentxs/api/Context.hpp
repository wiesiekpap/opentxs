// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CONTEXT_HPP
#define OPENTXS_API_CONTEXT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <functional>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Periodic.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client

namespace network
{
class Asio;
class ZAP;
}  // namespace network

namespace server
{
class Manager;
}  // namespace server

class Crypto;
class Primitives;
class Settings;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace rpc
{
namespace request
{
class Base;
}  // namespace request

namespace response
{
class Base;
}  // namespace response
}  // namespace rpc

class Options;
}  // namespace opentxs

class QObject;

namespace opentxs
{
namespace api
{
class OPENTXS_EXPORT Context : virtual public Periodic
{
public:
    using ShutdownCallback = std::function<void()>;

    static auto SuggestFolder(const std::string& app) noexcept -> std::string;

    virtual auto Asio() const noexcept -> const network::Asio& = 0;
    virtual auto Client(const int instance) const
        -> const api::client::Manager& = 0;
    virtual auto Clients() const -> std::size_t = 0;
    virtual auto Config(const std::string& path) const
        -> const api::Settings& = 0;
    virtual auto Crypto() const -> const api::Crypto& = 0;
    virtual auto Factory() const -> const api::Primitives& = 0;
    virtual void HandleSignals(ShutdownCallback* callback = nullptr) const = 0;
    virtual auto ProfileId() const -> std::string = 0;
    OPENTXS_NO_EXPORT virtual auto QtRootObject() const noexcept
        -> QObject* = 0;
    virtual auto RPC(const rpc::request::Base& command) const noexcept
        -> std::unique_ptr<rpc::response::Base> = 0;
    virtual auto RPC(const ReadView command, const AllocateOutput response)
        const noexcept -> bool = 0;
    /** Throws std::out_of_range if the specified server does not exist. */
    virtual auto Server(const int instance) const
        -> const api::server::Manager& = 0;
    virtual auto Servers() const -> std::size_t = 0;
    /** Start up a new client
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    virtual auto StartClient(const Options& args, const int instance) const
        -> const api::client::Manager& = 0;
    virtual auto StartClient(const int instance) const
        -> const api::client::Manager& = 0;
    virtual auto StartClient(
        const Options& args,
        const int instance,
        const std::string& recoverWords,
        const std::string& recoverPassphrase) const
        -> const api::client::Manager& = 0;
    /** Start up a new server
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    virtual auto StartServer(const Options& args, const int instance) const
        -> const api::server::Manager& = 0;
    virtual auto StartServer(const int instance) const
        -> const api::server::Manager& = 0;
    /** Access ZAP configuration API */
    virtual auto ZAP() const -> const api::network::ZAP& = 0;
    virtual auto ZMQ() const -> const opentxs::network::zeromq::Context& = 0;

    OPENTXS_NO_EXPORT ~Context() override = default;

protected:
    Context() = default;

private:
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;
};
}  // namespace api
}  // namespace opentxs
#endif

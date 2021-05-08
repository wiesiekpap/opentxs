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
class ThreadPool;
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
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class OPENTXS_EXPORT Context : virtual public Periodic
{
public:
    using ShutdownCallback = std::function<void()>;

    static std::string SuggestFolder(const std::string& app) noexcept;

    virtual const network::Asio& Asio() const noexcept = 0;
    virtual const api::client::Manager& Client(const int instance) const = 0;
    virtual std::size_t Clients() const = 0;
    virtual const api::Settings& Config(const std::string& path) const = 0;
    virtual const api::Crypto& Crypto() const = 0;
    virtual const api::Primitives& Factory() const = 0;
    virtual void HandleSignals(ShutdownCallback* callback = nullptr) const = 0;
    virtual std::string ProfileId() const = 0;
    virtual std::unique_ptr<rpc::response::Base> RPC(
        const rpc::request::Base& command) const noexcept = 0;
    virtual bool RPC(const ReadView command, const AllocateOutput response)
        const noexcept = 0;
    /** Throws std::out_of_range if the specified server does not exist. */
    virtual const api::server::Manager& Server(const int instance) const = 0;
    virtual std::size_t Servers() const = 0;
    /** Start up a new client
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    virtual const api::client::Manager& StartClient(
        const ArgList& args,
        const int instance) const = 0;
#if OT_CRYPTO_WITH_BIP32
    virtual const api::client::Manager& StartClient(
        const ArgList& args,
        const int instance,
        const std::string& recoverWords,
        const std::string& recoverPassphrase) const = 0;
#endif  // OT_CRYPTO_WITH_BIP32
    /** Start up a new server
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    virtual const api::server::Manager& StartServer(
        const ArgList& args,
        const int instance,
        const bool inproc = false) const = 0;
    virtual const api::ThreadPool& ThreadPool() const noexcept = 0;
    /** Access ZAP configuration API */
    virtual const api::network::ZAP& ZAP() const = 0;
    virtual const opentxs::network::zeromq::Context& ZMQ() const = 0;

    OPENTXS_NO_EXPORT ~Context() override = default;

protected:
    Context() = default;

private:
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif

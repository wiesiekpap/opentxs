// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CONTEXT_HPP
#define OPENTXS_API_CONTEXT_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <chrono>
#include <functional>
#include <string>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Periodic.hpp"

namespace opentxs
{
namespace api
{
class Primitives;
class ThreadPool;
}  // namespace api

namespace proto
{
class RPCCommand;
class RPCResponse;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class Context : virtual public Periodic
{
public:
    using ShutdownCallback = std::function<void()>;

    OPENTXS_EXPORT static std::string SuggestFolder(
        const std::string& app) noexcept;

    OPENTXS_EXPORT virtual const api::client::Manager& Client(
        const int instance) const = 0;
    OPENTXS_EXPORT virtual std::size_t Clients() const = 0;
    OPENTXS_EXPORT virtual const api::Settings& Config(
        const std::string& path) const = 0;
    OPENTXS_EXPORT virtual const api::Crypto& Crypto() const = 0;
    OPENTXS_EXPORT virtual const api::Primitives& Factory() const = 0;
    OPENTXS_EXPORT virtual void HandleSignals(
        ShutdownCallback* callback = nullptr) const = 0;
    OPENTXS_EXPORT virtual std::string ProfileId() const = 0;
    OPENTXS_EXPORT virtual proto::RPCResponse RPC(
        const proto::RPCCommand& command) const = 0;
    /** Throws std::out_of_range if the specified server does not exist. */
    OPENTXS_EXPORT virtual const api::server::Manager& Server(
        const int instance) const = 0;
    OPENTXS_EXPORT virtual std::size_t Servers() const = 0;
    /** Start up a new client
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    OPENTXS_EXPORT virtual const api::client::Manager& StartClient(
        const ArgList& args,
        const int instance) const = 0;
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual const api::client::Manager& StartClient(
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
    OPENTXS_EXPORT virtual const api::server::Manager& StartServer(
        const ArgList& args,
        const int instance,
        const bool inproc = false) const = 0;
    OPENTXS_EXPORT virtual const api::ThreadPool& ThreadPool()
        const noexcept = 0;
    /** Access ZAP configuration API */
    OPENTXS_EXPORT virtual const api::network::ZAP& ZAP() const = 0;
    OPENTXS_EXPORT virtual const opentxs::network::zeromq::Context& ZMQ()
        const = 0;

    OPENTXS_EXPORT ~Context() override = default;

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

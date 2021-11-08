// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <type_traits>

#include "api/session/Session.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/api/session/Client.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/ZMQ.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Context;
class Crypto;
class Legacy;
class Settings;

namespace crypto
{
class Blockchain;
}  // namespace crypto
}  // namespace api

namespace identifier
{
class Nym;
class Server;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

class Flag;
class Identifier;
class OTAPI_Exec;
class OT_API;
class Options;
}  // namespace opentxs

namespace opentxs::api::session::implementation
{
class Client final : public internal::Client, public Session
{
public:
    auto Activity() const -> const api::client::Activity& final;
    auto Contacts() const -> const api::client::Contacts& final;
    auto Exec(const std::string& wallet = "") const -> const OTAPI_Exec& final;
    auto InternalUI() const noexcept -> const client::internal::UI& final
    {
        return *ui_;
    }
    using Session::Lock;
    auto Lock(const identifier::Nym& nymID, const identifier::Server& serverID)
        const -> std::recursive_mutex& final;
    auto NewNym(const identifier::Nym& id) const noexcept -> void final;
    auto OTAPI(const std::string& wallet = "") const -> const OT_API& final;
    auto OTX() const -> const client::OTX& final;
    auto Pair() const -> const api::client::Pair& final;
    auto ServerAction() const -> const client::ServerAction& final;
    auto UI() const -> const api::client::UI& final;
    auto Workflow() const -> const client::Workflow& final;
    auto ZMQ() const -> const api::network::ZMQ& final;

    auto Init() -> void final;
    auto StartActivity() -> void;
    auto StartBlockchain() noexcept -> void;
    auto StartContacts() -> void;

    Client(
        const api::Context& parent,
        Flag& running,
        Options&& args,
        const api::Settings& config,
        const api::Crypto& crypto,
        const opentxs::network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);

    ~Client() final;

private:
    std::unique_ptr<network::ZMQ> zeromq_;
    std::unique_ptr<client::internal::Contacts> contacts_;
    std::unique_ptr<client::Activity> activity_;
    std::shared_ptr<crypto::Blockchain> blockchain_;
    std::unique_ptr<client::Workflow> workflow_;
    std::unique_ptr<OT_API> ot_api_;
    std::unique_ptr<OTAPI_Exec> otapi_exec_;
    std::unique_ptr<client::ServerAction> server_action_;
    std::unique_ptr<client::OTX> otx_;
    std::unique_ptr<client::internal::Pair> pair_;
    std::unique_ptr<client::internal::UI> ui_;
    mutable std::mutex map_lock_;
    mutable std::map<ContextID, std::recursive_mutex> context_locks_;

    auto get_lock(const ContextID context) const -> std::recursive_mutex&;

    void Cleanup();

    Client() = delete;
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    auto operator=(const Client&) -> Client& = delete;
    auto operator=(Client&&) -> Client& = delete;
};
}  // namespace opentxs::api::session::implementation

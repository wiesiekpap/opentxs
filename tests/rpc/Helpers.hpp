// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/contact/ContactItemType.hpp"
// IWYU pragma: no_include "opentxs/crypto/Language.hpp"
// IWYU pragma: no_include "opentxs/crypto/SeedStyle.hpp"

#pragma once

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "opentxs/contact/Types.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client

namespace server
{
class Manager;
}  // namespace server

class Context;
class Core;
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
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class RPCPushCounter
{
public:
    auto at(std::size_t index) const -> const zmq::Message&;
    auto size() const noexcept -> std::size_t;
    auto wait(std::size_t index) const noexcept -> bool;

    auto shutdown() noexcept -> void;

    RPCPushCounter(const ot::api::Context& ot) noexcept;

    ~RPCPushCounter();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    RPCPushCounter(const RPCPushCounter&) = delete;
    RPCPushCounter(RPCPushCounter&&) = delete;
    auto operator=(const RPCPushCounter&) -> RPCPushCounter& = delete;
    auto operator=(RPCPushCounter&&) -> RPCPushCounter& = delete;
};

class RPC_fixture : virtual public ::testing::Test
{
protected:
    using AccountMap = std::map<std::string, std::vector<std::string>>;
    using Amount = std::int64_t;
    using IssuedUnits = std::vector<std::string>;
    using LocalNymMap = std::map<int, std::set<std::string>>;
    using SeedMap = std::map<int, std::set<std::string>>;

    static SeedMap seed_map_;
    static LocalNymMap local_nym_map_;
    static IssuedUnits created_units_;
    static AccountMap registered_accounts_;

    const ot::api::Context& ot_;
    RPCPushCounter& push_;

    auto CreateNym(
        const ot::api::Core& api,
        const std::string& name,
        const std::string& seed,
        int index) const noexcept -> std::string;
    auto DepositCheques(
        const ot::api::client::Manager& api,
        const ot::api::server::Manager& server,
        const std::string& nym) const noexcept -> std::size_t;
    auto ImportBip39(const ot::api::Core& api, const std::string& words)
        const noexcept -> std::string;
    auto ImportServerContract(
        const ot::api::server::Manager& from,
        const ot::api::client::Manager& to) const noexcept -> bool;
    // TODO modify UIHelpers and put the Counter struct in a non-anonymous
    // namespace so that we can forward declare the type without "declared but
    // not defined" errors
    auto InitAccountActivityCounter(
        const ot::api::client::Manager& api,
        const std::string& nym,
        const std::string& account,
        void* counter) const noexcept -> void;
    auto IssueUnit(
        const ot::api::client::Manager& api,
        const ot::api::server::Manager& server,
        const std::string& issuer,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::string& fraction,
        std::uint32_t power,
        ot::contact::ContactItemType unitOfAccount) const noexcept
        -> std::string;
    auto RefreshAccount(
        const ot::api::client::Manager& api,
        const ot::identifier::Nym& nym,
        const ot::identifier::Server& server) const noexcept -> void;
    auto RefreshAccount(
        const ot::api::client::Manager& api,
        const std::vector<std::string> nyms,
        const ot::identifier::Server& server) const noexcept -> void;
    auto RegisterAccount(
        const ot::api::client::Manager& api,
        const ot::api::server::Manager& server,
        const std::string& nym,
        const std::string& unit,
        const std::string& label) const noexcept -> std::string;
    auto RegisterNym(
        const ot::api::client::Manager& api,
        const ot::api::server::Manager& server,
        const std::string& nym) const noexcept -> bool;
    auto SendCheque(
        const ot::api::client::Manager& api,
        const ot::api::server::Manager& server,
        const std::string& nym,
        const std::string& account,
        const std::string& contact,
        const std::string& memo,
        Amount amount) const noexcept -> bool;
    auto SendTransfer(
        const ot::api::client::Manager& api,
        const ot::api::server::Manager& server,
        const std::string& sender,
        const std::string& fromAccount,
        const std::string& toAccount,
        const std::string& memo,
        Amount amount) const noexcept -> bool;
    auto SetIntroductionServer(
        const ot::api::client::Manager& on,
        const ot::api::server::Manager& to) const noexcept -> bool;
    auto StartClient(int index) const noexcept
        -> const ot::api::client::Manager&;
    auto StartServer(int index) const noexcept
        -> const ot::api::server::Manager&;

    virtual auto Cleanup() noexcept -> void;

    RPC_fixture() noexcept;

private:
    auto init_maps(int instance) const noexcept -> void;
};
}  // namespace ottest

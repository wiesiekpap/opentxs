// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <string>
#include <tuple>

#include "Basic.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"

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
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace ottest
{
enum class Widget : int {
    AccountActivityUSD,
    AccountList,
    AccountSummaryBTC,
    AccountSummaryBCH,
    AccountSummaryUSD,
    ActivitySummary,
    ContactList,
    MessagableList,
    Profile,
    PayableListBTC,
    PayableListBCH,
    ActivityThreadAlice,
    ActivityThreadBob,
    ActivityThreadIssuer,
    ContactIssuer,
    AccountSummary,
};

using WidgetCallback = std::function<bool()>;
// target counter value, callback
using WidgetCallbackData = std::tuple<int, WidgetCallback, std::promise<bool>>;
// name, counter
using WidgetData = std::tuple<Widget, int, WidgetCallbackData>;
using WidgetMap = std::map<ot::OTIdentifier, WidgetData>;
using WidgetTypeMap = std::map<Widget, ot::OTIdentifier>;
using StateMap =
    std::map<std::string, std::map<Widget, std::map<int, WidgetCallback>>>;

struct Server {
    const ot::api::server::Manager* api_{nullptr};
    bool init_{false};
    const ot::OTServerID id_{ot::identifier::Server::Factory()};
    const std::string password_;

    auto Contract() const noexcept -> ot::OTServerContract;
    auto Reason() const noexcept -> ot::OTPasswordPrompt;

    auto init(const ot::api::server::Manager& api) noexcept -> void;
};

struct User {
    const std::string words_;
    const std::string passphrase_;
    const std::string name_;
    const ot::api::client::Manager* api_;
    bool init_;
    std::string seed_id_;
    std::uint32_t index_;
    ot::Nym_p nym_;
    ot::OTNymID nym_id_;
    std::string payment_code_;

    auto Account(const std::string& type) const noexcept
        -> const ot::Identifier&;
    auto Contact(const std::string& contact) const noexcept
        -> const ot::Identifier&;
    auto PaymentCode() const -> ot::OTPaymentCode;
    auto Reason() const noexcept -> ot::OTPasswordPrompt;
    auto SetAccount(const std::string& type, const std::string& id)
        const noexcept -> bool;
    auto SetAccount(const std::string& type, const ot::Identifier& id)
        const noexcept -> bool;
    auto SetContact(const std::string& contact, const std::string& id)
        const noexcept -> bool;
    auto SetContact(const std::string& contact, const ot::Identifier& id)
        const noexcept -> bool;

    auto init(
        const ot::api::client::Manager& api,
        const ot::contact::ContactItemType type =
            ot::contact::ContactItemType::Individual,
        const std::uint32_t index = 0,
        const ot::crypto::SeedStyle seed =
            ot::crypto::SeedStyle::BIP39) noexcept -> bool;
    auto init(
        const ot::api::client::Manager& api,
        const Server& server,
        const ot::contact::ContactItemType type =
            ot::contact::ContactItemType::Individual,
        const std::uint32_t index = 0,
        const ot::crypto::SeedStyle seed =
            ot::crypto::SeedStyle::BIP39) noexcept -> bool;
    auto init_custom(
        const ot::api::client::Manager& api,
        const Server& server,
        const std::function<void(User&)> custom,
        const ot::contact::ContactItemType type =
            ot::contact::ContactItemType::Individual,
        const std::uint32_t index = 0,
        const ot::crypto::SeedStyle seed =
            ot::crypto::SeedStyle::BIP39) noexcept -> void;

    User(
        const std::string words,
        const std::string name,
        const std::string passphrase = "") noexcept;

private:
    mutable std::mutex lock_;
    mutable std::map<std::string, ot::OTIdentifier> contacts_;
    mutable std::map<std::string, ot::OTIdentifier> accounts_;

    auto init_basic(
        const ot::api::client::Manager& api,
        const ot::contact::ContactItemType type,
        const std::uint32_t index,
        const ot::crypto::SeedStyle seed) noexcept -> bool;

    User(const User&) = delete;
    User(User&&) = delete;
    User& operator=(const User&) = delete;
    User& operator=(User&&) = delete;
};

struct Callbacks {
    mutable std::mutex callback_lock_;
    ot::OTZMQListenCallback callback_;

    auto Count() const noexcept -> std::size_t;

    auto RegisterWidget(
        const ot::Lock& callbackLock,
        const Widget type,
        const ot::Identifier& id,
        int counter = 0,
        WidgetCallback callback = {}) noexcept -> std::future<bool>;

    auto SetCallback(
        const Widget type,
        int limit,
        WidgetCallback callback) noexcept -> std::future<bool>;

    Callbacks(const std::string& name) noexcept;

private:
    mutable std::mutex map_lock_;
    const std::string name_;
    WidgetMap widget_map_;
    WidgetTypeMap ui_names_;

    auto callback(const ot::network::zeromq::Message& incoming) noexcept
        -> void;

    Callbacks() = delete;
};

struct Issuer {
    static const int expected_bailments_{3};
    static const std::string new_notary_name_;

    int bailment_counter_;
    std::promise<bool> bailment_promise_;
    std::shared_future<bool> bailment_;
    std::promise<bool> store_secret_promise_;
    std::shared_future<bool> store_secret_;

    Issuer() noexcept;
};

class IntegrationFixture : public ::testing::Test
{
public:
    static const User alex_;
    static const User bob_;
    static const User issuer_;
    static const User chris_;
    static const Server server_1_;
};

auto set_introduction_server(
    const ot::api::client::Manager& api,
    const Server& server) noexcept -> void;
auto test_future(
    std::future<bool>& future,
    const unsigned int seconds = 60) noexcept -> bool;
}  // namespace ottest

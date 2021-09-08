// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>
#include <utility>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace ottest
{
const User IntegrationFixture::alex_{
    "spike nominee miss inquiry fee nothing belt list other daughter leave "
    "valley twelve gossip paper",
    "Alex"};
const User IntegrationFixture::bob_{
    "trim thunder unveil reduce crop cradle zone inquiry anchor skate property "
    "fringe obey butter text tank drama palm guilt pudding laundry stay axis "
    "prosper",
    "Bob"};
const User IntegrationFixture::issuer_{
    "abandon abandon abandon abandon abandon abandon abandon abandon abandon "
    "abandon abandon about",
    "Issuer"};
const User IntegrationFixture::chris_{
    "abandon abandon abandon abandon abandon abandon abandon abandon abandon "
    "abandon abandon prosper",
    "Chris"};
const Server IntegrationFixture::server_1_{};

auto set_introduction_server(
    const ot::api::client::Manager& api,
    const Server& server) noexcept -> void
{
    auto bytes = ot::Space{};
    server.Contract()->Serialize(ot::writer(bytes), true);
    auto clientVersion = api.Wallet().Server(ot::reader(bytes));
    api.OTX().SetIntroductionServer(clientVersion);
}

auto test_future(std::future<bool>& future, const unsigned int seconds) noexcept
    -> bool
{
    const auto status =
        future.wait_until(ot::Clock::now() + std::chrono::seconds{seconds});

    if (std::future_status::ready == status) { return future.get(); }

    return false;
}

Callbacks::Callbacks(const std::string& name) noexcept
    : callback_lock_()
    , callback_(ot::network::zeromq::ListenCallback::Factory(
          [this](const auto& incoming) -> void { callback(incoming); }))
    , map_lock_()
    , name_(name)
    , widget_map_()
    , ui_names_()
{
}

auto Callbacks::callback(const ot::network::zeromq::Message& incoming) noexcept
    -> void
{
    ot::Lock lock(callback_lock_);
    const auto widgetID = ot::Identifier::Factory(incoming.Body().at(0));

    ASSERT_NE("", widgetID->str().c_str());

    auto& [type, counter, callbackData] = widget_map_.at(widgetID);
    auto& [limit, callback, future] = callbackData;
    ++counter;

    if (counter >= limit) {
        if (callback) {
            future.set_value(callback());
            callback = {};
            future = {};
            limit = 0;
        } else {
            ot::LogOutput("::Callbacks::")(__func__)(": ")(
                name_)(" missing callback for ")(static_cast<int>(type))
                .Flush();
        }
    } else {
        ot::LogVerbose("::Callbacks::")(__func__)(": Skipping update ")(
            counter)(" to ")(static_cast<int>(type))
            .Flush();
    }
}

auto Callbacks::Count() const noexcept -> std::size_t
{
    ot::Lock lock(map_lock_);

    return widget_map_.size();
}

auto Callbacks::RegisterWidget(
    const ot::Lock& callbackLock,
    const Widget type,
    const ot::Identifier& id,
    int counter,
    WidgetCallback callback) noexcept -> std::future<bool>
{
    ot::LogDetail("::Callbacks::")(__func__)(": Name: ")(name_)(" ID: ")(id)
        .Flush();
    WidgetData data{};
    std::get<0>(data) = type;
    auto& [limit, cb, promise] = std::get<2>(data);
    limit = counter, cb = callback, promise = {};
    auto output = promise.get_future();
    widget_map_.emplace(id, std::move(data));
    ui_names_.emplace(type, id);

    OT_ASSERT(widget_map_.size() == ui_names_.size());

    return output;
}

auto Callbacks::SetCallback(
    const Widget type,
    int limit,
    WidgetCallback callback) noexcept -> std::future<bool>
{
    ot::Lock lock(map_lock_);
    auto& [counter, cb, promise] =
        std::get<2>(widget_map_.at(ui_names_.at(type)));
    counter += limit;
    cb = callback;
    promise = {};

    return promise.get_future();
}

Issuer::Issuer() noexcept
    : bailment_counter_(0)
    , bailment_promise_()
    , bailment_(bailment_promise_.get_future())
    , store_secret_promise_()
    , store_secret_(store_secret_promise_.get_future())
{
}

auto Server::Contract() const noexcept -> ot::OTServerContract
{
    return api_->Wallet().Server(id_);
}

auto Server::Reason() const noexcept -> ot::OTPasswordPrompt
{
    OT_ASSERT(nullptr != api_);

    return api_->Factory().PasswordPrompt(__func__);
}

auto Server::init(const ot::api::server::Manager& api) noexcept -> void
{
    if (init_) { return; }

    api_ = &api;
    const_cast<ot::OTServerID&>(id_) = api.ID();

    {
        const auto section = ot::String::Factory("permissions");
        const auto key = ot::String::Factory("admin_password");
        auto value = ot::String::Factory();
        auto exists{false};
        api.Config().Check_str(section, key, value, exists);

        OT_ASSERT(exists);

        const_cast<std::string&>(password_) = value->Get();
    }

    OT_ASSERT(false == id_->empty());
    OT_ASSERT(false == password_.empty());

    init_ = true;
}

User::User(
    const std::string words,
    const std::string name,
    const std::string passphrase) noexcept
    : words_(words)
    , passphrase_(passphrase)
    , name_(name)
    , api_(nullptr)
    , init_(false)
    , seed_id_()
    , index_()
    , nym_()
    , nym_id_(ot::identifier::Nym::Factory())
    , payment_code_()
    , lock_()
    , contacts_()
    , accounts_()
{
}

auto User::Account(const std::string& type) const noexcept
    -> const ot::Identifier&
{
    ot::Lock lock(lock_);

    return accounts_.at(type).get();
}

auto User::Contact(const std::string& contact) const noexcept
    -> const ot::Identifier&
{
    ot::Lock lock(lock_);

    return contacts_.at(contact).get();
}

auto User::init_basic(
    const ot::api::client::Manager& api,
    const ot::contact::ContactItemType type,
    const std::uint32_t index,
    const ot::crypto::SeedStyle seed) noexcept -> bool
{
    if (init_) { return false; }

    api_ = &api;
    seed_id_ = api.Seeds().ImportSeed(
        api.Factory().SecretFromText(words_),
        api.Factory().SecretFromText(passphrase_),
        seed,
        ot::crypto::Language::en,
        Reason());
    index_ = index;
    nym_ = api.Wallet().Nym(
        Reason(), name_, {seed_id_, static_cast<int>(index_)}, type);

    OT_ASSERT(nym_);

    nym_id_ = nym_->ID();
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    payment_code_ =
        api.Factory()
            .PaymentCode(
                seed_id_, index_, ot::PaymentCode::DefaultVersion, Reason())
            ->asBase58();
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

    return true;
}

auto User::init(
    const ot::api::client::Manager& api,
    const Server& server,
    const ot::contact::ContactItemType type,
    const std::uint32_t index,
    const ot::crypto::SeedStyle seed) noexcept -> bool
{
    if (init_basic(api, type, index, seed)) {
        set_introduction_server(api, server);
        init_ = true;

        return true;
    }

    return false;
}

auto User::init(
    const ot::api::client::Manager& api,
    const ot::contact::ContactItemType type,
    const std::uint32_t index,
    const ot::crypto::SeedStyle seed) noexcept -> bool
{
    if (init_basic(api, type, index, seed)) {
        init_ = true;

        return true;
    }

    return false;
}

auto User::init_custom(
    const ot::api::client::Manager& api,
    const Server& server,
    const std::function<void(User&)> custom,
    const ot::contact::ContactItemType type,
    const std::uint32_t index,
    const ot::crypto::SeedStyle seed) noexcept -> void
{
    if (init(api, server, type, index, seed) && custom) { custom(*this); }
}

auto User::PaymentCode() const -> ot::OTPaymentCode
{
    OT_ASSERT(nullptr != api_);

    return api_->Factory().PaymentCode(payment_code_);
}

auto User::Reason() const noexcept -> ot::OTPasswordPrompt
{
    OT_ASSERT(nullptr != api_);

    return api_->Factory().PasswordPrompt(__func__);
}

auto User::SetAccount(const std::string& type, const std::string& id)
    const noexcept -> bool
{
    OT_ASSERT(nullptr != api_);

    return SetAccount(type, api_->Factory().Identifier(id));
}

auto User::SetAccount(const std::string& type, const ot::Identifier& id)
    const noexcept -> bool
{
    OT_ASSERT(nullptr != api_);

    ot::Lock lock(lock_);
    const auto [it, added] = accounts_.emplace(type, id);

    return added;
}

auto User::SetContact(const std::string& contact, const std::string& id)
    const noexcept -> bool
{
    OT_ASSERT(nullptr != api_);

    return SetContact(contact, api_->Factory().Identifier(id));
}

auto User::SetContact(const std::string& contact, const ot::Identifier& id)
    const noexcept -> bool
{
    OT_ASSERT(nullptr != api_);

    ot::Lock lock(lock_);
    const auto [it, added] = contacts_.emplace(contact, id);

    return added;
}
}  // namespace ottest

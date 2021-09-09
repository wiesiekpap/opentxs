// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "api/client/Manager.hpp"  // IWYU pragma: associated

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>

#include "2_Factory.hpp"
#include "api/Core.hpp"
#include "api/Scheduler.hpp"
#include "api/StorageParent.hpp"
#include "core/Shutdown.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/api/client/Factory.hpp"
#include "internal/api/network/Factory.hpp"
#include "internal/api/network/Network.hpp"
#include "internal/api/storage/Storage.hpp"
#include "opentxs/api/Options.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"

#define OT_METHOD "opentxs::api::client::implementation::Manager::"

namespace opentxs::factory
{
auto ClientManager(
    const api::internal::Context& parent,
    Flag& running,
    Options&& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance) noexcept -> std::unique_ptr<api::client::Manager>
{
    using ReturnType = api::client::implementation::Manager;

    return std::make_unique<ReturnType>(
        parent,
        running,
        std::move(args),
        config,
        crypto,
        context,
        dataFolder,
        instance);
}
}  // namespace opentxs::factory

namespace opentxs::api::client::implementation
{
Manager::Manager(
    const api::internal::Context& parent,
    Flag& running,
    Options&& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const opentxs::network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance)
    : client::internal::Manager()
    , Core(
          parent,
          running,
          std::move(args),
          crypto,
          config,
          context,
          dataFolder,
          instance,
          [&](const auto& zmq, const auto& endpoints, auto& config) {
              return factory::NetworkAPI(
                  *this,
                  parent.Asio(),
                  zmq,
                  endpoints,
                  factory::BlockchainNetworkAPI(*this, endpoints, zmq),
                  config,
                  false);
          },
          std::unique_ptr<api::internal::Factory>{
              factory::FactoryAPIClient(*this)})
    , zeromq_(opentxs::Factory::ZMQ(*this, running_))
    , contacts_(factory::ContactAPI(*this))
    , activity_(factory::ActivityAPI(*this, *contacts_))
    , blockchain_(factory::BlockchainAPI(
          *this,
          *activity_,
          *contacts_,
          parent_.Legacy(),
          dataFolder,
          args_))
    , workflow_(factory::Workflow(*this, *activity_, *contacts_))
    , ot_api_(new OT_API(
          *this,
          *activity_,
          *contacts_,
          *workflow_,
          *zeromq_,
          std::bind(&Manager::get_lock, this, std::placeholders::_1)))
    , otapi_exec_(new OTAPI_Exec(
          *this,
          *activity_,
          *contacts_,
          *zeromq_,
          *ot_api_,
          std::bind(&Manager::get_lock, this, std::placeholders::_1)))
    , server_action_(factory::ServerAction(
          *this,
          std::bind(&Manager::get_lock, this, std::placeholders::_1)))
    , otx_(factory::OTX(
          running_,
          *this,
          *ot_api_->m_pClient,
          std::bind(&Manager::get_lock, this, std::placeholders::_1)))
    , pair_(factory::PairAPI(running_, *this))
    , ui_(factory::UI(*this, *blockchain_, running_))
    , map_lock_()
    , context_locks_()
{
    wallet_.reset(factory::Wallet(*this));

    OT_ASSERT(wallet_);
    OT_ASSERT(zeromq_);
    OT_ASSERT(contacts_);
    OT_ASSERT(activity_);
    OT_ASSERT(blockchain_);
    OT_ASSERT(workflow_);
    OT_ASSERT(ot_api_);
    OT_ASSERT(otapi_exec_);
    OT_ASSERT(server_action_);
    OT_ASSERT(otx_);
    OT_ASSERT(ui_);
    OT_ASSERT(pair_);

    network_.Blockchain().Internal().Init(
        *blockchain_, parent_.Legacy(), dataFolder, args_);
}

auto Manager::Activity() const -> const api::client::Activity&
{
    OT_ASSERT(activity_)

    return *activity_;
}

auto Manager::Blockchain() const -> const api::client::Blockchain&
{
    OT_ASSERT(blockchain_)

    return *blockchain_;
}

void Manager::Cleanup()
{
    LogDetail(OT_METHOD)(__func__)(": Shutting down and cleaning up.").Flush();
    shutdown_sender_.Activate();
    ui_->Shutdown();
    ui_.reset();
    pair_.reset();
    otx_.reset();
    server_action_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
    workflow_.reset();
#if OT_BLOCKCHAIN
    network_.Blockchain().Internal().Shutdown();
    contacts_->prepare_shutdown();
#endif  // OT_BLOCKCHAIN
    blockchain_.reset();
    activity_.reset();
    contacts_.reset();
    zeromq_.reset();
    Core::cleanup();
}

auto Manager::Contacts() const -> const api::client::Contacts&
{
    OT_ASSERT(contacts_)

    return *contacts_;
}

auto Manager::get_lock(const ContextID context) const -> std::recursive_mutex&
{
    opentxs::Lock lock(map_lock_);

    return context_locks_[context];
}

auto Manager::Exec(const std::string&) const -> const OTAPI_Exec&
{
    OT_ASSERT(otapi_exec_);

    return *otapi_exec_;
}

auto Manager::Init() -> void
{
#if OT_BLOCKCHAIN
    contacts_->init(blockchain_);
#endif  // OT_BLOCKCHAIN

#if OT_CRYPTO_WITH_BIP32
    OT_ASSERT(seeds_)
#endif  // OT_CRYPTO_WITH_BIP32

    StorageParent::init(
        factory_
#if OT_CRYPTO_WITH_BIP32
        ,
        *seeds_
#endif  // OT_CRYPTO_WITH_BIP32
    );
    StartContacts();
    StartActivity();
    pair_->init();
    blockchain_->Init();
    StartBlockchain();
    ui_->Init();
}

auto Manager::Lock(
    const identifier::Nym& nymID,
    const identifier::Server& serverID) const -> std::recursive_mutex&
{
    return get_lock({nymID.str(), serverID.str()});
}

auto Manager::NewNym(const identifier::Nym& id) const noexcept -> void
{
    Core::NewNym(id);
    blockchain_->NewNym(id);
}

auto Manager::OTAPI(const std::string&) const -> const OT_API&
{
    OT_ASSERT(ot_api_);

    return *ot_api_;
}

auto Manager::OTX() const -> const api::client::OTX&
{
    OT_ASSERT(otx_);

    return *otx_;
}

auto Manager::Pair() const -> const api::client::Pair&
{
    OT_ASSERT(pair_);

    return *pair_;
}

auto Manager::ServerAction() const -> const api::client::ServerAction&
{
    OT_ASSERT(server_action_);

    return *server_action_;
}

void Manager::StartActivity()
{
    Scheduler::Start(storage_.get(), network_.DHT());
}

auto Manager::StartBlockchain() noexcept -> void
{
#if OT_BLOCKCHAIN
    for (const auto chain : args_.DisabledBlockchains()) {
        network_.Blockchain().Disable(chain);
    }

    network_.Blockchain().Internal().RestoreNetworks();
#endif  // OT_BLOCKCHAIN
}

void Manager::StartContacts()
{
    OT_ASSERT(contacts_);

    contacts_->start();
}

auto Manager::UI() const -> const api::client::UI&
{
    OT_ASSERT(ui_)

    return *ui_;
}

auto Manager::Workflow() const -> const api::client::Workflow&
{
    OT_ASSERT(workflow_);

    return *workflow_;
}

auto Manager::ZMQ() const -> const api::network::ZMQ&
{
    OT_ASSERT(zeromq_);

    return *zeromq_;
}

Manager::~Manager()
{
    running_.Off();
    Cleanup();
}
}  // namespace opentxs::api::client::implementation

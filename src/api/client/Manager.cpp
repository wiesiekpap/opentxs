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
#include "internal/api/storage/Storage.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
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
    const ArgList& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance) -> api::client::internal::Manager*
{
    return new api::client::implementation::Manager(
        parent, running, args, config, crypto, context, dataFolder, instance);
}
}  // namespace opentxs::factory

namespace opentxs::api::client::implementation
{
Manager::Manager(
    const api::internal::Context& parent,
    Flag& running,
    const ArgList& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const opentxs::network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance)
    : client::internal::Manager()
    , Core(
          parent,
          running,
          args,
          crypto,
          config,
          context,
          dataFolder,
          instance,
          false,
          std::unique_ptr<api::internal::Factory>{
              factory::FactoryAPIClient(*this)})
    , zeromq_(opentxs::Factory::ZMQ(*this, running_))
    , contacts_(factory::ContactAPI(*this))
    , activity_(factory::Activity(*this, *contacts_))
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
    , ui_(factory::UI(
          *this,
#if OT_BLOCKCHAIN
          *blockchain_,
#endif  // OT_BLOCKCHAIN
          running_))
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
    LogDetail(OT_METHOD)(__FUNCTION__)(": Shutting down and cleaning up.")
        .Flush();
    shutdown_sender_.Activate();
    ui_->Shutdown();
    ui_.reset();
    pair_.reset();
    otx_.reset();
    server_action_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
    workflow_.reset();
    blockchain_->Shutdown();
#if OT_BLOCKCHAIN
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

auto Manager::init(
    const api::Legacy& legacy,
    const internal::Manager& parent,
    const std::string& dataFolder,
    const ArgList& args,
    std::shared_ptr<internal::Blockchain>& blockchain,
    std::unique_ptr<internal::Contacts>& contacts) noexcept
    -> std::unique_ptr<internal::Activity>
{
    contacts = factory::ContactAPI(parent);

    OT_ASSERT(contacts);

    auto activity = factory::Activity(parent, *contacts);

    OT_ASSERT(activity);

    blockchain = factory::BlockchainAPI(
        parent, *activity, *contacts, legacy, dataFolder, args);

    OT_ASSERT(blockchain);

    return activity;
}

void Manager::Init()
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
#if OT_BLOCKCHAIN
    StartBlockchain();
#endif  // OT_BLOCKCHAIN
    ui_->Init();
}

auto Manager::Lock(
    const identifier::Nym& nymID,
    const identifier::Server& serverID) const -> std::recursive_mutex&
{
    return get_lock({nymID.str(), serverID.str()});
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
    OT_ASSERT(dht_)

    Scheduler::Start(storage_.get(), dht_.get());
}

#if OT_BLOCKCHAIN
auto Manager::StartBlockchain() noexcept -> void
{
    try {
        for (const auto& s : args_.at(OPENTXS_ARG_DISABLED_BLOCKCHAINS)) {
            try {
                const auto chain = static_cast<opentxs::blockchain::Type>(
                    static_cast<std::uint32_t>(std::stoul(s)));
                blockchain_->Disable(chain);
            } catch (...) {
                continue;
            }
        }
    } catch (...) {
    }

    blockchain_->RestoreNetworks();
}
#endif  // OT_BLOCKCHAIN

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

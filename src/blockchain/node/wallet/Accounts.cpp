// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/node/wallet/Accounts.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <utility>

#include "blockchain/node/wallet/subchain/NotificationStateData.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/HDPath.pb.h"
#include "util/LMDB.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(AccountsJobs job) noexcept -> std::string_view
{
    static const auto map = Map<AccountsJobs, CString>{
        {AccountsJobs::shutdown, "shutdown"},
        {AccountsJobs::nym, "nym"},
        {AccountsJobs::header, "header"},
        {AccountsJobs::reorg, "reorg"},
        {AccountsJobs::filter, "filter"},
        {AccountsJobs::mempool, "mempool"},
        {AccountsJobs::block, "block"},
        {AccountsJobs::job_finished, "job_finished"},
        {AccountsJobs::reorg_begin_ack, "reorg_begin_ack"},
        {AccountsJobs::reorg_end_ack, "reorg_end_ack"},
        {AccountsJobs::init, "init"},
        {AccountsJobs::key, "key"},
        {AccountsJobs::statemachine, "statemachine"},
    };

    return map.at(job);
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Accounts::Imp::Imp(
    Accounts& parent,
    const api::Session& api,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    const std::string_view shutdown,
    const std::string_view endpoint,
    CString&& publish,
    CString&& pull,
    allocator_type alloc) noexcept
    : Actor(
          api,
          0ms,
          batch,
          alloc,
          {
              {CString{shutdown, alloc}, Direction::Connect},
              {CString{
                   api.Crypto().Blockchain().Internal().KeyEndpoint(),
                   alloc},
               Direction::Connect},
              {CString{api.Endpoints().BlockchainBlockAvailable(), alloc},
               Direction::Connect},
              {CString{api.Endpoints().BlockchainMempool(), alloc},
               Direction::Connect},
              {CString{api.Endpoints().BlockchainNewFilter(), alloc},
               Direction::Connect},
              {CString{api.Endpoints().BlockchainReorg(), alloc},
               Direction::Connect},
              {CString{api.Endpoints().NymCreated(), alloc},
               Direction::Connect},
          },
          {
              {pull, Direction::Bind},
          },
          {},
          {
              {network::zeromq::socket::Type::Pair,
               {
                   {CString{endpoint}, Direction::Connect},
               }},
              {network::zeromq::socket::Type::Publish,
               {
                   {publish, Direction::Bind},
               }},
          })
    , parent_(parent)
    , api_(api)
    , node_(node)
    , db_(db)
    , mempool_(mempool)
    , task_finished_([&](const Identifier& id, const char* type) {
        auto work = MakeWork(Work::job_finished);
        work.AddFrame(id.data(), id.size());
        work.AddFrame(CString(type));  // TODO allocator
        pipeline_.Push(std::move(work));
    })
    , chain_(chain)
    , filter_type_(node_.FilterOracleInternal().DefaultType())
    , shutdown_endpoint_(shutdown, alloc)
    , publish_endpoint_(std::move(publish))
    , pull_endpoint_(std::move(pull))
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , to_accounts_(pipeline_.Internal().ExtraSocket(1))
    , state_(State::normal)
    , job_counter_()
    , pc_counter_(job_counter_.Allocate())
    , reorg_(std::nullopt)
    , accounts_(alloc)
    , notification_channels_(alloc)
{
}

Accounts::Imp::Imp(
    Accounts& parent,
    const api::Session& api,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    const std::string_view shutdown,
    const std::string_view endpoint,
    allocator_type alloc) noexcept
    : Imp(parent,
          api,
          node,
          db,
          mempool,
          batch,
          chain,
          shutdown,
          endpoint,
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          std::move(alloc))
{
}

auto Accounts::Imp::do_reorg() noexcept -> void
{
    finish_background_tasks();
    const auto& reorg = reorg_.value();
    auto errors = std::atomic_int{0};

    {
        auto headerOracleLock =
            Lock{node_.HeaderOracle().Internal().GetMutex()};
        auto tx = db_.StartReorg();

        for (auto& [nym, data] : accounts_) {
            data.ProcessReorg(headerOracleLock, tx, errors, reorg.ancestor_);
        }

        for (auto& [code, data] : notification_channels_) {
            data.ProcessReorg(headerOracleLock, tx, errors, reorg.ancestor_);
        }

        finish_background_tasks();

        if (false == db_.FinalizeReorg(tx, reorg.ancestor_)) { ++errors; }

        try {
            if (0 < errors) { throw std::runtime_error{"Prepare step failed"}; }

            if (false == tx.Finalize(true)) {

                throw std::runtime_error{"Finalize transaction failed"};
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(": ")(e.what())
                .Flush();

            OT_FAIL;
        }
    }

    try {
        if (false == db_.AdvanceTo(reorg.tip_)) {

            throw std::runtime_error{"Advance chain failed"};
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(" ")(e.what())
            .Flush();

        OT_FAIL;
    }

    // TODO ensure filter oracle has processed the reorg before proceeding
    state_ = State::post_reorg;

    if (0u < reorg_children()) {
        to_accounts_.Send(MakeWork(AccountJobs::reorg_end));
    } else {
        finish_reorg();
    }
}

auto Accounts::Imp::do_shutdown() noexcept -> void
{
    to_accounts_.Send(MakeWork(WorkType::Shutdown));

    for (auto& [code, data] : notification_channels_) { data.Shutdown(); }

    notification_channels_.clear();
    accounts_.clear();
}

auto Accounts::Imp::finish_background_tasks() noexcept -> void
{
    for (auto& [code, data] : notification_channels_) {
        data.FinishBackgroundTasks();
    }
}

auto Accounts::Imp::finish_reorg() noexcept -> void
{
    const auto& tip = reorg_->tip_;
    LogConsole()(DisplayString(chain_))(": reorg to ")(tip.second->asHex())(
        " at height ")(tip.first)(" finished")
        .Flush();
    reorg_ = std::nullopt;
    disable_automatic_processing_ = false;
    state_ = State::normal;
}

auto Accounts::Imp::index_nym(const identifier::Nym& id) noexcept -> void
{
    const auto pNym = api_.Wallet().Nym(id);

    OT_ASSERT(pNym);

    const auto& nym = *pNym;
    auto code = api_.Factory().PaymentCode(nym.PaymentCode());

    if (3 > code.Version()) { return; }

    LogConsole()("Initializing payment code ")(code.asBase58())(" on ")(
        DisplayString(chain_))
        .Flush();
    auto accountID = NotificationStateData::calculate_id(api_, chain_, code);
    notification_channels_.try_emplace(
        std::move(accountID),
        api_,
        node_,
        parent_,
        db_,
        task_finished_,
        pc_counter_,
        filter_type_,
        chain_,
        id,
        std::move(code),
        [&] {
            auto out = proto::HDPath{};
            nym.PaymentCodePath(out);

            return out;
        }());
}

auto Accounts::Imp::Init(boost::shared_ptr<Imp> me) noexcept -> void
{
    signal_startup(me);
}

auto Accounts::Imp::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::post_reorg: {
            state_post_reorg(work, std::move(msg));
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Accounts::Imp::process_block(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto hash = api_.Factory().Data(body.at(2));
    process_block(hash);
}

auto Accounts::Imp::process_block(const block::Hash& block) noexcept -> void
{
    for (auto& [code, data] : notification_channels_) {
        data.ProcessBlockAvailable(block);
    }
}

auto Accounts::Imp::process_block_header(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    if (3 >= body.size()) {
        LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
            ": invalid message")
            .Flush();

        OT_FAIL;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto position = block::Position{
        body.at(3).as<block::Height>(),
        api_.Factory().Data(body.at(2).Bytes())};
    db_.AdvanceTo(position);
}

auto Accounts::Imp::process_filter(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto type = body.at(2).as<filter::Type>();

    if (type != node_.FilterOracleInternal().DefaultType()) { return; }

    const auto position = block::Position{
        body.at(3).as<block::Height>(), api_.Factory().Data(body.at(4))};
    process_filter(position);
}

auto Accounts::Imp::process_filter(const block::Position& tip) noexcept -> void
{
    for (auto& [code, data] : notification_channels_) {
        data.ProcessNewFilter(tip);
    }
}

auto Accounts::Imp::process_job_finished(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto id = api_.Factory().Identifier(body.at(1));
    const auto str = CString{body.at(2).Bytes(), get_allocator()};
    process_job_finished(id, str.c_str());
}

auto Accounts::Imp::process_job_finished(
    const Identifier& id,
    const char* type) noexcept -> void
{
    for (auto& [code, data] : notification_channels_) {
        data.ProcessTaskComplete(id, type);
    }
}

auto Accounts::Imp::process_key(Message&& in) noexcept -> void
{
    for (auto& [code, data] : notification_channels_) { data.ProcessKey(); }
}

auto Accounts::Imp::process_mempool(Message&& in) noexcept -> void
{
    const auto body = in.Body();
    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    if (auto tx = mempool_.Query(body.at(2).Bytes()); tx) {
        process_mempool(std::move(tx));
    }
}

auto Accounts::Imp::process_mempool(
    std::shared_ptr<const block::bitcoin::Transaction>&& tx) noexcept -> void
{
    for (auto& [code, data] : notification_channels_) {
        data.ProcessMempool(tx);
    }
}

auto Accounts::Imp::process_nym(const identifier::Nym& nym) noexcept -> bool
{
    auto [it, added] = accounts_.try_emplace(
        nym,
        parent_,
        api_,
        api_.Crypto().Blockchain().Account(nym, chain_),
        node_,
        db_,
        mempool_,
        chain_,
        filter_type_,
        shutdown_endpoint_,
        publish_endpoint_,
        pull_endpoint_,
        job_counter_.Allocate());

    if (added) {
        LogConsole()("Initializing ")(DisplayString(chain_))(" wallet for ")(
            nym)
            .Flush();
    }

    index_nym(nym);

    return added;
}

auto Accounts::Imp::process_nym(Message&& in) noexcept -> bool
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto id = api_.Factory().NymID(body.at(1));

    if (0 == id->size()) { return false; }

    return process_nym(id);
}

auto Accounts::Imp::reorg_children() const noexcept -> std::size_t
{
    // FIXME also include notification_channels_.size() once SubchainStateData
    // uses Actor

    return accounts_.size();
}

auto Accounts::Imp::startup() noexcept -> void
{
    for (const auto& id : api_.Wallet().LocalNyms()) { process_nym(id); }

    do_work();
}

auto Accounts::Imp::state_normal(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(false == reorg_.has_value());

    switch (work) {
        case Work::nym: {
            process_nym(std::move(msg));
        } break;
        case Work::header: {
            process_block_header(std::move(msg));
        } break;
        case Work::reorg: {
            transition_state_reorg(std::move(msg));
        } break;
        case Work::filter: {
            process_filter(std::move(msg));
        } break;
        case Work::mempool: {
            process_mempool(std::move(msg));
        } break;
        case Work::block: {
            process_block(std::move(msg));
        } break;
        case Work::job_finished: {
            process_job_finished(std::move(msg));
        } break;
        case Work::key: {
            process_key(std::move(msg));
        } break;
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack: {
            LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
                ": received reorg status while in normal state")
                .Flush();

            OT_FAIL;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
                ": unhandled type")
                .Flush();

            OT_FAIL;
        }
    }
}

auto Accounts::Imp::state_post_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(reorg_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::nym:
        case Work::header:
        case Work::reorg:
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::job_finished:
        case Work::key:
        case Work::statemachine: {
            // NOTE defer processing of non-reorg messages until after reorg is
            // complete
            pipeline_.Push(std::move(msg));
        } break;
        case Work::reorg_end_ack: {
            transition_state_normal(std::move(msg));
        } break;
        case Work::reorg_begin_ack: {
            LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
                ": received reorg_begin_ack status while in reorg state")
                .Flush();

            OT_FAIL;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
                ": unhandled type")
                .Flush();

            OT_FAIL;
        }
    }
}

auto Accounts::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    OT_ASSERT(reorg_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::nym:
        case Work::header:
        case Work::reorg:
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::job_finished:
        case Work::key:
        case Work::statemachine: {
            // NOTE defer processing of non-reorg messages until after reorg is
            // complete
            pipeline_.Push(std::move(msg));
        } break;
        case Work::reorg_begin_ack: {
            transition_state_post_reorg(std::move(msg));
        } break;
        case Work::reorg_end_ack: {
            LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
                ": received reorg_end_ack status while in reorg state")
                .Flush();

            OT_FAIL;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
                ": unhandled type")
                .Flush();

            OT_FAIL;
        }
    }
}

auto Accounts::Imp::transition_state_normal(Message&& in) noexcept -> void
{
    OT_ASSERT(0u < reorg_children());

    auto& reorg = reorg_.value();
    const auto& target = reorg.target_;
    auto& counter = reorg.done_;
    ++counter;

    if (counter < target) {

        return;
    } else if (counter == target) {
        finish_reorg();
    } else {

        OT_FAIL;
    }
}

auto Accounts::Imp::transition_state_post_reorg(Message&& in) noexcept -> void
{
    OT_ASSERT(0u < reorg_children());

    auto& reorg = reorg_.value();
    const auto& target = reorg.target_;
    auto& counter = reorg.ready_;
    ++counter;

    if (counter < target) {

        return;
    } else if (counter == target) {
        do_reorg();
    } else {

        OT_FAIL;
    }
}

auto Accounts::Imp::transition_state_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    if (6 > body.size()) {
        LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
            ": invalid message")
            .Flush();

        OT_FAIL;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    reorg_.emplace(
        block::Position{
            body.at(3).as<block::Height>(),
            api_.Factory().Data(body.at(2).Bytes())},
        block::Position{
            body.at(5).as<block::Height>(),
            api_.Factory().Data(body.at(4).Bytes())},
        reorg_children());
    LogConsole()("Processing ")(DisplayString(chain_))(" reorg to ")(
        reorg_->tip_.second->asHex())(" at height ")(reorg_->tip_.first)
        .Flush();
    disable_automatic_processing_ = true;
    state_ = State::reorg;

    if (0u < reorg_children()) {
        to_accounts_.Send(MakeWork(AccountJobs::reorg_begin));
    } else {
        do_reorg();
    }
}

auto Accounts::Imp::work() noexcept -> bool
{
    auto output{false};

    for (auto& [code, data] : notification_channels_) {
        output |= data.ProcessStateMachine();
    }

    to_accounts_.Send(MakeWork(OT_ZMQ_STATE_MACHINE_SIGNAL));

    return output;
}

Accounts::Imp::~Imp() = default;
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Accounts::Accounts(
    const api::Session& api,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain,
    const std::string_view shutdown,
    const std::string_view endpoint) noexcept
    : imp_([&] {
        const auto& asio = api.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)},
            *this,
            api,
            node,
            db,
            mempool,
            batchID,
            chain,
            shutdown,
            endpoint);
    }())
{
    OT_ASSERT(imp_);

    imp_->Init(imp_);
}

Accounts::~Accounts() { imp_->Shutdown(); }
}  // namespace opentxs::blockchain::node::wallet

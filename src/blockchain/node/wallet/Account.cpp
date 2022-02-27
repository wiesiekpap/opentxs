// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/node/wallet/Account.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <chrono>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(AccountJobs job) noexcept -> std::string_view
{
    static const auto map = Map<AccountJobs, CString>{
        {AccountJobs::shutdown, "shutdown"},
        {AccountJobs::filter, "filter"},
        {AccountJobs::mempool, "mempool"},
        {AccountJobs::block, "block"},
        {AccountJobs::job_finished, "job_finished"},
        {AccountJobs::reorg_begin, "reorg_begin"},
        {AccountJobs::reorg_begin_ack, "reorg_begin_ack"},
        {AccountJobs::reorg_end, "reorg_end"},
        {AccountJobs::reorg_end_ack, "reorg_end_ack"},
        {AccountJobs::init, "init"},
        {AccountJobs::key, "key"},
        {AccountJobs::statemachine, "statemachine"},
    };

    return map.at(job);
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Account::Imp::Imp(
    Accounts& parent,
    const api::Session& api,
    const crypto::Account& account,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    const filter::Type filter,
    const std::string_view shutdown,
    const std::string_view publish,
    const std::string_view pull,
    Outstanding&& jobs,
    allocator_type alloc) noexcept
    : Actor(
          api,
          0ms,
          batch,
          alloc,
          {
              {CString{shutdown, alloc}, Direction::Connect},
              {CString{publish, alloc}, Direction::Connect},
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
          },
          {},
          {},
          {
              {network::zeromq::socket::Type::Push,
               {
                   {CString{pull}, Direction::Connect},
               }},
          })
    , parent_(parent)
    , api_(api)
    , account_(account)
    , node_(node)
    , db_(db)
    , mempool_(mempool)
    , chain_(chain)
    , filter_type_(node_.FilterOracleInternal().DefaultType())
    , task_finished_([&](const Identifier& id, const char* type) {
        auto work = MakeWork(Work::job_finished);
        work.AddFrame(id.data(), id.size());
        work.AddFrame(CString(type));  // TODO allocator
        pipeline_.Push(std::move(work));
    })
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , state_(State::normal)
    , rng_(std::random_device{}())
    , internal_(alloc)
    , external_(alloc)
    , outgoing_(alloc)
    , incoming_(alloc)
    , jobs_(std::move(jobs))
{
}

auto Account::Imp::do_shutdown() noexcept -> void
{
    for_each_subchain([&](auto& s) { s.Shutdown(); });
    internal_.clear();
    external_.clear();
    outgoing_.clear();
    incoming_.clear();
}

auto Account::Imp::finish_background_tasks() noexcept -> void
{
    for_each_account([](auto& s) { s.FinishBackgroundTasks(); });
}

auto Account::Imp::get(
    const crypto::Deterministic& account,
    const Subchain subchain,
    Subchains& map) noexcept -> DeterministicStateData&
{
    auto it = map.find(account.ID());

    if (map.end() != it) { return it->second; }

    return instantiate(account, subchain, map);
}

auto Account::Imp::Init(boost::shared_ptr<Imp> me) noexcept -> void
{
    signal_startup(me);
}

auto Account::Imp::instantiate(
    const crypto::Deterministic& account,
    const Subchain subchain,
    Subchains& map) noexcept -> DeterministicStateData&
{
    auto [it, added] = map.try_emplace(
        account.ID(),
        api_,
        node_,
        parent_,
        db_,
        account,
        task_finished_,
        jobs_,
        filter_type_,
        subchain);

    OT_ASSERT(added);

    return it->second;
}

auto Account::Imp::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Account::Imp::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& parent) noexcept -> bool
{
    auto output{false};

    for_each_subchain([&](auto& s) {
        output |= s.ProcessReorg(headerOracleLock, tx, errors, parent);
    });

    return output;
}

auto Account::Imp::process_block(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto hash = api_.Factory().Data(body.at(2));
    process_block(hash);
}

auto Account::Imp::process_block(const block::Hash& block) noexcept -> void
{
    for_each_subchain([&](auto& s) { s.ProcessBlockAvailable(block); });
}

auto Account::Imp::process_filter(Message&& in) noexcept -> void
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

auto Account::Imp::process_filter(const block::Position& tip) noexcept -> void
{
    for_each_subchain([&](auto& s) { s.ProcessNewFilter(tip); });
}

auto Account::Imp::process_key(Message&& in) noexcept -> void
{
    for_each_subchain([&](auto& s) { s.ProcessKey(); });
}

auto Account::Imp::process_job_finished(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto id = api_.Factory().Identifier(body.at(1));
    const auto str = CString{body.at(2).Bytes(), get_allocator()};
    process_job_finished(id, str.c_str());
}

auto Account::Imp::process_job_finished(
    const Identifier& id,
    const char* type) noexcept -> void
{
    for_each_subchain([&](auto& s) { s.ProcessTaskComplete(id, type); });
}

auto Account::Imp::process_mempool(Message&& in) noexcept -> void
{
    const auto body = in.Body();
    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    if (auto tx = mempool_.Query(body.at(2).Bytes()); tx) {
        process_mempool(std::move(tx));
    }
}

auto Account::Imp::process_mempool(
    std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept -> void
{
    for_each_subchain([&](auto& s) { s.ProcessMempool(tx); });
}

auto Account::Imp::startup() noexcept -> void
{
    api_.Wallet().Internal().PublishNym(account_.NymID());

    for (const auto& account : account_.GetHD()) {
        instantiate(account, Subchain::Internal, internal_);
        instantiate(account, Subchain::External, external_);
    }

    for (const auto& account : account_.GetPaymentCode()) {
        instantiate(account, Subchain::Outgoing, outgoing_);
        instantiate(account, Subchain::Incoming, incoming_);
    }
}

auto Account::Imp::state_normal(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
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
        case Work::reorg_begin: {
            finish_background_tasks();
            disable_automatic_processing_ = true;
            state_ = State::reorg;
            to_parent_.Send(MakeWork(AccountsJobs::reorg_begin_ack));
        } break;
        case Work::reorg_end: {
            LogError()(OT_PRETTY_CLASS())(
                ": received reorg_end status while in normal state")
                .Flush();

            OT_FAIL;
        }
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::shutdown:
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
        case Work::reorg_end: {
            disable_automatic_processing_ = false;
            state_ = State::normal;
            to_parent_.Send(MakeWork(AccountsJobs::reorg_end_ack));
        } break;
        case Work::reorg_begin: {
            LogError()(OT_PRETTY_CLASS())(
                ": received reorg_begin status while in reorg state")
                .Flush();

            OT_FAIL;
        }
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::work() noexcept -> bool
{
    auto output{false};

    for_each_subchain([&](auto& s) { output |= s.ProcessStateMachine(); });

    return output;
}

Account::Imp::~Imp() { signal_shutdown(); }
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Account::Account(
    Accounts& parent,
    const api::Session& api,
    const crypto::Account& account,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain,
    const filter::Type filter,
    const std::string_view shutdown,
    const std::string_view publish,
    const std::string_view pull,
    Outstanding&& jobs) noexcept
    : imp_([&] {
        const auto& asio = api.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)},
            parent,
            api,
            account,
            node,
            db,
            mempool,
            batchID,
            chain,
            filter,
            shutdown,
            publish,
            pull,
            std::move(jobs));
    }())
{
    OT_ASSERT(imp_);

    imp_->Init(imp_);
}

Account::Account(Account&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    OT_ASSERT(imp_);
}

auto Account::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& parent) noexcept -> bool
{
    return imp_->ProcessReorg(headerOracleLock, tx, errors, parent);
}

Account::~Account() { imp_->Shutdown(); }
}  // namespace opentxs::blockchain::node::wallet

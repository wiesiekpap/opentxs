// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/node/wallet/Accounts.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <utility>

#include "blockchain/node/wallet/Account.hpp"
#include "blockchain/node/wallet/NotificationStateData.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/util/LogMacros.hpp"
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
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/HDPath.pb.h"
#include "util/Gatekeeper.hpp"
#include "util/Work.hpp"

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
    allocator_type alloc) noexcept
    : Actor(
          api,
          0ms,
          batch,
          alloc,
          {
              {CString{shutdown, alloc},
               network::zeromq::socket::Direction::Connect},
              {CString{
                   api.Crypto().Blockchain().Internal().KeyEndpoint(),
                   alloc},
               network::zeromq::socket::Direction::Connect},
              {CString{api.Endpoints().BlockchainBlockAvailable(), alloc},
               network::zeromq::socket::Direction::Connect},
              {CString{api.Endpoints().BlockchainMempool(), alloc},
               network::zeromq::socket::Direction::Connect},
              {CString{api.Endpoints().BlockchainNewFilter(), alloc},
               network::zeromq::socket::Direction::Connect},
              {CString{api.Endpoints().BlockchainReorg(), alloc},
               network::zeromq::socket::Direction::Connect},
              {CString{api.Endpoints().NymCreated(), alloc},
               network::zeromq::socket::Direction::Connect},
          },
          {},
          {},
          {
              {network::zeromq::socket::Type::Pair,
               {
                   {CString{endpoint},
                    network::zeromq::socket::Direction::Connect},
               }},
          })
    , parent_(parent)
    , api_(api)
    , node_(node)
    , db_(db)
    , mempool_(mempool)
    , task_finished_([this](const Identifier& id, const char* type) {
        auto work = MakeWork(Work::job_finished);
        work.AddFrame(id.data(), id.size());
        work.AddFrame(UnallocatedCString(type));
        pipeline_.Push(std::move(work));
    })
    , chain_(chain)
    , filter_type_(node_.FilterOracleInternal().DefaultType())
    , to_wallet_(pipeline_.Internal().ExtraSocket(0))
    , rng_(std::random_device{}())
    , job_counter_()
    , map_(alloc)
    , pc_counter_(job_counter_.Allocate())
    , payment_codes_(alloc)
{
}

auto Accounts::Imp::do_shutdown() noexcept -> void
{
    for_each([&](auto& a) { a.Shutdown(); });
    payment_codes_.clear();
    map_.clear();
}

auto Accounts::Imp::finish_background_tasks() noexcept -> void
{
    for_each([](auto& a) { a.FinishBackgroundTasks(); });
}

auto Accounts::Imp::for_each(
    std::function<void(wallet::Actor&)> action) noexcept -> void
{
    if (!action) { return; }

    auto actors = UnallocatedVector<wallet::Actor*>{};

    for (auto& [nym, account] : map_) { actors.emplace_back(&account); }

    for (auto& [code, account] : payment_codes_) {
        actors.emplace_back(&account);
    }

    std::shuffle(actors.begin(), actors.end(), rng_);

    for (auto* actor : actors) { action(*actor); }
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
    payment_codes_.try_emplace(
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
    const auto body = msg.Body();

    switch (work) {
        case Work::nym: {
            OT_ASSERT(1 < body.size());

            process_nym(body.at(1));
        } break;
        case Work::header: {
            process_block_header(std::move(msg));
        } break;
        case Work::reorg: {
            process_reorg(std::move(msg));
            // TODO ensure filter oracle has processed the reorg before running
            // state machine again
            do_work();
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
        default: {
            LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
                ": unhandled type")
                .Flush();

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
    for_each([&](auto& a) { a.ProcessBlockAvailable(block); });
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
    for_each([&](auto& a) { a.ProcessNewFilter(tip); });
}

auto Accounts::Imp::process_job_finished(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto id = api_.Factory().Identifier(body.at(1));
    const auto str = CString{body.at(2).Bytes()};
    process_job_finished(id, str.c_str());
}

auto Accounts::Imp::process_job_finished(
    const Identifier& id,
    const char* type) noexcept -> void
{
    for_each([&](auto& a) { a.ProcessTaskComplete(id, type); });
}

auto Accounts::Imp::process_key(Message&& in) noexcept -> void
{
    for_each([](auto& a) { a.ProcessKey(); });
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
    for_each([&](auto& a) { a.ProcessMempool(tx); });
}

auto Accounts::Imp::process_nym(const identifier::Nym& nym) noexcept -> bool
{
    auto [it, added] = map_.try_emplace(
        nym,
        api_,
        api_.Crypto().Blockchain().Account(nym, chain_),
        node_,
        parent_,
        db_,
        filter_type_,
        job_counter_.Allocate(),
        task_finished_);

    if (added) {
        LogConsole()("Initializing ")(DisplayString(chain_))(" wallet for ")(
            nym)
            .Flush();
    }

    index_nym(nym);

    return added;
}

auto Accounts::Imp::process_nym(const network::zeromq::Frame& message) noexcept
    -> bool
{
    const auto id = api_.Factory().NymID(message);

    if (0 == id->size()) { return false; }

    return process_nym(id);
}

auto Accounts::Imp::process_reorg(Message&& in) noexcept -> void
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

    const auto ancestor = block::Position{
        body.at(3).as<block::Height>(),
        api_.Factory().Data(body.at(2).Bytes())};
    const auto tip = block::Position{
        body.at(5).as<block::Height>(),
        api_.Factory().Data(body.at(4).Bytes())};
    LogConsole()(DisplayString(chain_))(": processing reorg to ")(
        tip.second->asHex())(" at height ")(tip.first)
        .Flush();
    finish_background_tasks();
    auto errors = std::atomic_int{};
    {
        auto headerOracleLock =
            Lock{node_.HeaderOracle().Internal().GetMutex()};
        auto tx = db_.StartReorg();
        process_reorg(headerOracleLock, tx, errors, ancestor);
        finish_background_tasks();

        if (false == db_.FinalizeReorg(tx, ancestor)) { ++errors; }

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
        if (false == db_.AdvanceTo(tip)) {

            throw std::runtime_error{"Advance chain failed"};
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(" ")(e.what())
            .Flush();

        OT_FAIL;
    }

    LogConsole()(DisplayString(chain_))(": reorg to ")(tip.second->asHex())(
        " at height ")(tip.first)(" finished")
        .Flush();
}

auto Accounts::Imp::process_reorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& parent) noexcept -> bool
{
    auto ticket = gatekeeper_.get();

    if (ticket) { return false; }

    auto output{false};

    for_each([&](auto& a) {
        output |= a.ProcessReorg(headerOracleLock, tx, errors, parent);
    });

    return output;
}

auto Accounts::Imp::startup() noexcept -> void
{
    for (const auto& id : api_.Wallet().LocalNyms()) { process_nym(id); }

    do_work();
}

auto Accounts::Imp::state_machine() noexcept -> bool
{
    auto output{false};

    for (auto& [code, account] : payment_codes_) {
        output |= account.ProcessStateMachine();
    }

    for (auto& [nym, account] : map_) {
        output |= account.ProcessStateMachine();
    }

    return output;
}

auto Accounts::Imp::Shutdown() noexcept -> void { signal_shutdown(); }

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

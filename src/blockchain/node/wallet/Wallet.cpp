// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/Wallet.hpp"  // IWYU pragma: associated

#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <utility>

#include "core/Worker.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::factory
{
auto BlockchainWallet(
    const api::Session& api,
    const blockchain::node::internal::Network& parent,
    blockchain::node::internal::WalletDatabase& db,
    const blockchain::node::internal::Mempool& mempool,
    const blockchain::Type chain,
    const std::string_view shutdown)
    -> std::unique_ptr<blockchain::node::internal::Wallet>
{
    using ReturnType = blockchain::node::implementation::Wallet;

    return std::make_unique<ReturnType>(
        api, parent, db, mempool, chain, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::wallet
{
auto lock_for_reorg(
    const std::string_view name,
    std::timed_mutex& mutex) noexcept -> std::unique_lock<std::timed_mutex>
{
    auto lock = std::unique_lock<std::timed_mutex>{mutex, std::defer_lock};
    auto failures{-1};

    while (false == lock.owns_lock()) {
        if (++failures < 300) {
            lock.try_lock_for(997ms);
        } else {
            LogError()(name)(" state machine is not responding").Flush();
            lock.try_lock_for(997ms);
        }
    }

    return lock;
}

auto print(WalletJobs job) noexcept -> std::string_view
{
    try {
        using Job = WalletJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::init, "init"},
            {Job::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid WalletJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::implementation
{
Wallet::Wallet(
    const api::Session& api,
    const node::internal::Network& parent,
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain,
    const std::string_view shutdown) noexcept
    : Worker(api, 10ms)
    , parent_(parent)
    , db_(db)
    , chain_(chain)
    , fee_oracle_(factory::FeeOracle(api_, chain))
    , accounts_(api, parent_, db_, mempool, chain_)
    , proposals_(api, parent_, db_, chain_)
{
    init_executor({
        UnallocatedCString{shutdown},
    });
}

auto Wallet::ConstructTransaction(
    const proto::BlockchainTransactionProposal& tx,
    std::promise<SendOutcome>&& promise) const noexcept -> void
{
    proposals_.Add(tx, std::move(promise));
    trigger();
}
auto Wallet::GetTransactions() const noexcept -> UnallocatedVector<block::pTxid>
{
    return db_.GetTransactions();
}

auto Wallet::GetBalance() const noexcept -> Balance { return db_.GetBalance(); }

auto Wallet::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    return db_.GetBalance(owner);
}

auto Wallet::GetBalance(const identifier::Nym& owner, const Identifier& node)
    const noexcept -> Balance
{
    return db_.GetBalance(owner, node);
}

auto Wallet::GetBalance(const crypto::Key& key) const noexcept -> Balance
{
    return db_.GetBalance(key);
}

auto Wallet::GetOutputs(alloc::Resource* alloc) const noexcept -> Vector<UTXO>
{
    return GetOutputs(TxoState::All, alloc);
}

auto Wallet::GetOutputs(TxoState type, alloc::Resource* alloc) const noexcept
    -> Vector<UTXO>
{
    return db_.GetOutputs(type, alloc);
}

auto Wallet::GetOutputs(const identifier::Nym& owner, alloc::Resource* alloc)
    const noexcept -> Vector<UTXO>
{
    return GetOutputs(owner, TxoState::All, alloc);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    TxoState type,
    alloc::Resource* alloc) const noexcept -> Vector<UTXO>
{
    return db_.GetOutputs(owner, type, alloc);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const Identifier& subaccount,
    alloc::Resource* alloc) const noexcept -> Vector<UTXO>
{
    return GetOutputs(owner, subaccount, TxoState::All, alloc);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const Identifier& node,
    TxoState type,
    alloc::Resource* alloc) const noexcept -> Vector<UTXO>
{
    return db_.GetOutputs(owner, node, type, alloc);
}

auto Wallet::GetOutputs(
    const crypto::Key& key,
    TxoState type,
    alloc::Resource* alloc) const noexcept -> Vector<UTXO>
{
    return db_.GetOutputs(key, type, alloc);
}

auto Wallet::GetTags(const block::Outpoint& output) const noexcept
    -> UnallocatedSet<TxoTag>
{
    return db_.GetOutputTags(output);
}

auto Wallet::Height() const noexcept -> block::Height
{
    return db_.GetWalletHeight();
}

auto Wallet::Init() noexcept -> void
{
    accounts_.Init();
    trigger();
}

auto Wallet::pipeline(zmq::Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(": invalid message")
            .Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();
    const auto start = Clock::now();

    switch (work) {
        case Work::shutdown: {
            protect_shutdown([this] { shut_down(); });
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(": unhandled type")
                .Flush();

            OT_FAIL;
        }
    }

    static constexpr auto limit = 1s;
    const auto elapsed = std::chrono::nanoseconds{Clock::now() - start};

    if (elapsed > limit) {
        LogTrace()(OT_PRETTY_CLASS())(print(chain_))(": spent ")(
            elapsed)(" processing ")(print(work))(" signal")
            .Flush();
    }
}

auto Wallet::shut_down() noexcept -> void
{
    LogDetail()("Shutting down ")(print(chain_))(" wallet").Flush();
    accounts_.Shutdown();
    close_pipeline();
    fee_oracle_.Shutdown();
    // TODO MT-34 investigate what other actions might be needed
}

auto Wallet::StartRescan() const noexcept -> bool
{
    accounts_.Rescan();

    return true;
}

auto Wallet::state_machine() noexcept -> bool
{
    if (false == running_.load()) { return false; }

    return proposals_.Run();
}

Wallet::~Wallet()
{
    protect_shutdown([this] { shut_down(); });
}
}  // namespace opentxs::blockchain::node::implementation

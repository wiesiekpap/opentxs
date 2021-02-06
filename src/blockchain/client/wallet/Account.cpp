// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/client/Wallet.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <map>
#include <optional>
#include <queue>
#include <type_traits>
#include <utility>
#include <vector>

#include "blockchain/client/wallet/HDStateData.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/api/client/blockchain/HD.hpp"
#include "opentxs/api/client/blockchain/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::client::implementation::Wallet::Account::"

namespace opentxs::blockchain::client::implementation
{
Wallet::Account::Account(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const BalanceTree& ref,
    const internal::Network& network,
    const internal::WalletDatabase& db,
    const zmq::socket::Push& threadPool,
    Outstanding&& jobs,
    const SimpleCallback& taskFinished) noexcept
    : api_(api)
    , blockchain_(blockchain)
    , ref_(ref)
    , network_(network)
    , db_(db)
    , filter_type_(network.FilterOracleInternal().DefaultType())
    , thread_pool_(threadPool)
    , task_finished_(taskFinished)
    , internal_()
    , external_()
    , jobs_(std::move(jobs))
{
    for (const auto& subaccount : ref_.GetHD()) {
        const auto& id = subaccount.ID();
        internal_.try_emplace(
            id,
            api_,
            blockchain_,
            network_,
            db_,
            subaccount,
            task_finished_,
            jobs_,
            filter_type_,
            Subchain::Internal);
        external_.try_emplace(
            id,
            api_,
            blockchain_,
            network_,
            db_,
            subaccount,
            task_finished_,
            jobs_,
            filter_type_,
            Subchain::External);
    }
}

Wallet::Account::Account(Account&& rhs) noexcept
    : api_(rhs.api_)
    , blockchain_(rhs.blockchain_)
    , ref_(rhs.ref_)
    , network_(rhs.network_)
    , db_(rhs.db_)
    , filter_type_(rhs.filter_type_)
    , thread_pool_(rhs.thread_pool_)
    , task_finished_(rhs.task_finished_)
    , internal_(std::move(rhs.internal_))
    , external_(std::move(rhs.external_))
    , jobs_(std::move(rhs.jobs_))
{
}

auto Wallet::Account::queue_work(
    const Task task,
    const HDStateData& data) noexcept -> void
{
    while (jobs_.limited()) { Sleep(std::chrono::microseconds(100)); }

    using Pool = internal::ThreadPool;

    auto work = Pool::MakeWork(api_, network_.Chain(), Pool::Work::HDAccount);
    work->AddFrame(task);
    work->AddFrame(reinterpret_cast<std::uintptr_t>(&data));
    thread_pool_.Send(work);
    ++jobs_;
}

auto Wallet::Account::reorg(const block::Position& parent) noexcept -> bool
{
    auto output{false};

    for (const auto& subaccount : ref_.GetHD()) {
        const auto& id = subaccount.ID();
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Processing account ")(id)
            .Flush();

        {
            auto it = internal_.find(id);

            if (internal_.end() == it) {
                auto [it2, added] = internal_.try_emplace(
                    id,
                    api_,
                    blockchain_,
                    network_,
                    db_,
                    subaccount,
                    task_finished_,
                    jobs_,
                    filter_type_,
                    Subchain::Internal);
                it = it2;
            }

            output |= reorg_hd(it->second, parent);
        }

        {
            auto it = external_.find(id);

            if (external_.end() == it) {
                auto [it2, added] = external_.try_emplace(
                    id,
                    api_,
                    blockchain_,
                    network_,
                    db_,
                    subaccount,
                    task_finished_,
                    jobs_,
                    filter_type_,
                    Subchain::External);
                it = it2;
            }

            output |= reorg_hd(it->second, parent);
        }
    }

    return output;
}

auto Wallet::Account::reorg_hd(
    HDStateData& data,
    const block::Position& parent) noexcept -> bool
{
    return data.reorg_.Queue(parent);
}

auto Wallet::Account::state_machine() noexcept -> bool
{
    auto output{false};

    for (const auto& subaccount : ref_.GetHD()) {
        const auto& id = subaccount.ID();
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Processing account ")(id)
            .Flush();

        {
            auto it = internal_.find(id);

            if (internal_.end() == it) {
                auto [it2, added] = internal_.try_emplace(
                    id,
                    api_,
                    blockchain_,
                    network_,
                    db_,
                    subaccount,
                    task_finished_,
                    jobs_,
                    filter_type_,
                    Subchain::Internal);
                it = it2;
            }

            output |= state_machine_hd(it->second);
        }

        {
            auto it = external_.find(id);

            if (external_.end() == it) {
                auto [it2, added] = external_.try_emplace(
                    id,
                    api_,
                    blockchain_,
                    network_,
                    db_,
                    subaccount,
                    task_finished_,
                    jobs_,
                    filter_type_,
                    Subchain::External);
                it = it2;
            }

            output |= state_machine_hd(it->second);
        }
    }

    return output;
}

auto Wallet::Account::state_machine_hd(HDStateData& data) noexcept -> bool
{
    const auto& node = data.node_;
    const auto subchain = data.subchain_;
    auto& reorg = data.reorg_;
    auto& running = data.running_;
    auto& lastIndexed = data.last_indexed_;
    auto& lastScanned = data.last_scanned_;
    auto& requestBlocks = data.blocks_to_request_;
    auto& outstanding = data.outstanding_blocks_;
    auto& queue = data.process_block_queue_;

    if (running) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Task is running").Flush();

        return false;
    }

    if (false == reorg.Empty()) {
        running.store(true);
        queue_work(Task::reorg, data);
        LogTrace(OT_METHOD)(__FUNCTION__)(": Reorg queued").Flush();

        return false;
    }

    {
        for (const auto& hash : requestBlocks) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Requesting block ")(
                hash->asHex())(" queue position: ")(outstanding.size())
                .Flush();

            if (0 == outstanding.count(hash)) {
                auto [it, added] = outstanding.emplace(
                    hash, network_.BlockOracle().LoadBitcoin(hash));

                OT_ASSERT(added);

                queue.push(it);
            }
        }

        requestBlocks.clear();
    }

    {
        lastIndexed =
            db_.SubchainLastIndexed(node.ID(), subchain, filter_type_);
        const auto generated = node.LastGenerated(subchain);

        if (generated.has_value()) {
            if ((false == lastIndexed.has_value()) ||
                (lastIndexed.value() != generated.value())) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(": Subchain has ")(
                    generated.value() + 1)(" keys generated, but only ")(
                    lastIndexed.value_or(0))(" have been indexed.")
                    .Flush();
                running.store(true);
                queue_work(Task::index, data);
                LogTrace(OT_METHOD)(__FUNCTION__)(": Index queued").Flush();

                return false;
            } else {
                LogVerbose(OT_METHOD)(__FUNCTION__)(": All ")(
                    generated.value() + 1)(" generated keys have been indexed.")
                    .Flush();
            }
        }
    }

    {
        auto needScan{false};

        if (lastScanned.has_value()) {
            const auto bestFilter =
                network_.FilterOracleInternal().FilterTip(filter_type_);

            if (lastScanned == bestFilter) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(
                    ": Subchain has been scanned to the newest downloaded "
                    "filter ")(bestFilter.second->asHex())(" at height ")(
                    bestFilter.first)
                    .Flush();
            } else {
                const auto [ancestor, best] =
                    network_.HeaderOracleInternal().CommonParent(
                        lastScanned.value());
                lastScanned = ancestor;

                if (lastScanned == best) {
                    LogVerbose(OT_METHOD)(__FUNCTION__)(
                        ": Subchain has been scanned to current best block ")(
                        best.second->asHex())(" at height ")(best.first)
                        .Flush();
                } else {
                    needScan = true;
                    LogVerbose(OT_METHOD)(__FUNCTION__)(
                        ": Subchain scanning progress: ")(
                        lastScanned.value().first)
                        .Flush();
                }
            }
        } else {
            needScan = true;
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Subchain scanning progress: ")(0)
                .Flush();
        }

        if (needScan) {
            running.store(true);
            queue_work(Task::scan, data);
            LogTrace(OT_METHOD)(__FUNCTION__)(": Scan queued").Flush();

            return false;
        }
    }

    if (false == queue.empty()) {
        const auto& [id, future] = *queue.front();

        if (std::future_status::ready ==
            future.wait_for(std::chrono::milliseconds(1))) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Ready to process block")
                .Flush();
            running.store(true);
            queue_work(Task::process, data);
            LogTrace(OT_METHOD)(__FUNCTION__)(": Process queued").Flush();

            return false;
        } else {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Waiting for block download")
                .Flush();
        }
    }

    return false;
}
}  // namespace opentxs::blockchain::client::implementation

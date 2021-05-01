// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "1_Internal.hpp"               // IWYU pragma: associated
#include "blockchain/node/Mempool.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <algorithm>
#include <chrono>
#include <queue>
#include <set>
#include <shared_mutex>
#include <string_view>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node
{
struct Mempool::Imp {
    auto Dump() const noexcept -> std::set<std::string>
    {
        auto lock = sLock{lock_};

        return active_;
    }
    auto Query(ReadView txid) const noexcept
        -> std::shared_ptr<const block::bitcoin::Transaction>
    {
        auto lock = sLock{lock_};

        try {

            return transactions_.at(Hash{txid});
        } catch (...) {

            return {};
        }
    }
    auto Submit(ReadView txid) const noexcept -> bool
    {
        const auto input = std::vector<ReadView>{txid};
        const auto output = Submit(input);

        return output.front();
    }
    auto Submit(const std::vector<ReadView>& txids) const noexcept
        -> std::vector<bool>
    {
        auto output = std::vector<bool>{};
        output.reserve(txids.size());
        auto lock = eLock{lock_};

        for (const auto& txid : txids) {
            const auto [it, added] =
                transactions_.try_emplace(Hash{txid}, nullptr);

            if (added) {
                unexpired_txid_.emplace(Clock::now(), txid);
                output.emplace_back(true);
            } else {
                output.emplace_back(false);
            }
        }

        OT_ASSERT(output.size() == txids.size());

        return output;
    }
    auto Submit(std::unique_ptr<const block::bitcoin::Transaction> tx)
        const noexcept -> void
    {
        if (!tx) { return; }

        auto txid = Hash{tx->ID().Bytes()};
        auto lock = eLock{lock_};
        const auto [it, added] = transactions_.try_emplace(txid, nullptr);

        if (added) { unexpired_txid_.emplace(Clock::now(), txid); }

        auto& existing = it->second;

        if (!existing) {
            existing = std::move(tx);
            notify(txid);
            active_.emplace(txid);
            unexpired_tx_.emplace(Clock::now(), std::move(txid));
        }
    }

    auto Heartbeat() noexcept -> void
    {
        const auto now = Clock::now();
        auto lock = eLock{lock_};

        while (0 < unexpired_tx_.size()) {
            const auto& [time, txid] = unexpired_tx_.front();

            if ((now - time) < tx_limit_) { break; }

            try {
                transactions_.at(txid).reset();
            } catch (...) {
            }

            active_.erase(txid);
            unexpired_tx_.pop();
        }

        while (0 < unexpired_txid_.size()) {
            const auto& [time, txid] = unexpired_txid_.front();

            if ((now - time) < txid_limit_) { break; }

            transactions_.erase(txid);
            active_.erase(txid);
            unexpired_txid_.pop();
        }
    }

    Imp(const api::Core& api,
        const network::zeromq::socket::Publish& socket,
        const Type chain) noexcept
        : api_(api)
        , chain_(chain)
        , lock_()
        , transactions_()
        , active_()
        , unexpired_txid_()
        , unexpired_tx_()
        , socket_(socket)
    {
    }

private:
    using Hash = std::string;
    using TransactionMap = robin_hood::unordered_flat_map<
        Hash,
        std::shared_ptr<const block::bitcoin::Transaction>>;
    using Data = std::pair<Time, Hash>;
    using Cache = std::queue<Data>;

    static constexpr auto tx_limit_ = std::chrono::hours{1};
    static constexpr auto txid_limit_ = std::chrono::hours{24};

    const api::Core& api_;
    const Type chain_;
    mutable std::shared_mutex lock_;
    mutable TransactionMap transactions_;
    mutable std::set<Hash> active_;
    mutable Cache unexpired_txid_;
    mutable Cache unexpired_tx_;
    const network::zeromq::socket::Publish& socket_;

    auto notify(ReadView txid) const noexcept -> void
    {
        auto work = api_.Network().ZeroMQ().TaggedMessage(
            WorkType::BlockchainMempoolUpdated);
        work->AddFrame(chain_);
        work->AddFrame(txid.data(), txid.size());
        socket_.Send(work);
    }
};

Mempool::Mempool(
    const api::Core& api,
    const network::zeromq::socket::Publish& socket,
    const Type chain) noexcept
    : imp_(std::make_unique<Imp>(api, socket, chain))
{
}

auto Mempool::Dump() const noexcept -> std::set<std::string>
{
    return imp_->Dump();
}

auto Mempool::Heartbeat() noexcept -> void { imp_->Heartbeat(); }

auto Mempool::Query(ReadView txid) const noexcept
    -> std::shared_ptr<const block::bitcoin::Transaction>
{
    return imp_->Query(txid);
}

auto Mempool::Submit(ReadView txid) const noexcept -> bool
{
    return imp_->Submit(txid);
}

auto Mempool::Submit(const std::vector<ReadView>& txids) const noexcept
    -> std::vector<bool>
{
    return imp_->Submit(txids);
}

auto Mempool::Submit(std::unique_ptr<const block::bitcoin::Transaction> tx)
    const noexcept -> void
{
    imp_->Submit(std::move(tx));
}

Mempool::~Mempool() = default;
}  // namespace opentxs::blockchain::node

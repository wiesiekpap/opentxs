// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/node/wallet/Wallet.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <iterator>
#include <memory>
#include <utility>

#include "core/Worker.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#define OT_METHOD "opentxs::blockchain::node::implementation::Wallet::"

namespace opentxs::factory
{
auto BlockchainWallet(
    const api::Core& api,
    const api::client::internal::Blockchain& crypto,
    const blockchain::node::internal::Network& parent,
    const blockchain::node::internal::WalletDatabase& db,
    const blockchain::node::internal::Mempool& mempool,
    const blockchain::Type chain,
    const std::string& shutdown)
    -> std::unique_ptr<blockchain::node::internal::Wallet>
{
    using ReturnType = blockchain::node::implementation::Wallet;

    return std::make_unique<ReturnType>(
        api, crypto, parent, db, mempool, chain, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::implementation
{
Wallet::Wallet(
    const api::Core& api,
    const api::client::internal::Blockchain& crypto,
    const node::internal::Network& parent,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain,
    const std::string& shutdown) noexcept
    : Worker(api, std::chrono::milliseconds(10))
    , parent_(parent)
    , db_(db)
    , mempool_(mempool)
    , crypto_(crypto)
    , chain_(chain)
    , task_finished_([this]() { trigger(); })
    , enabled_(false)
    , accounts_(api, crypto_, parent_, db_, chain_, task_finished_)
    , proposals_(api, crypto_, parent_, db_, chain_)
{
    init_executor({
        shutdown,
        api.Endpoints().BlockchainReorg(),
        api.Endpoints().NymCreated(),
        api.Endpoints().InternalBlockchainFilterUpdated(chain),
        crypto_.KeyEndpoint(),
        api.Endpoints().BlockchainMempool(),
    });
}

auto Wallet::ConstructTransaction(
    const proto::BlockchainTransactionProposal& tx,
    std::promise<SendOutcome>&& promise) const noexcept -> void
{
    proposals_.Add(tx, std::move(promise));
    trigger();
}

auto Wallet::convert(const DBUTXOs& in) const noexcept -> std::vector<UTXO>
{
    auto out = std::vector<UTXO>{};
    out.reserve(in.size());
    std::transform(
        in.begin(),
        in.end(),
        std::back_inserter(out),
        [this](const auto& utxo) {
            const auto& [outpoint, proto] = utxo;
            auto converted = UTXO{
                outpoint,
                factory::BitcoinTransactionOutput(
                    api_, crypto_, chain_, proto)};

            OT_ASSERT(converted.second);

            return converted;
        });

    return out;
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

auto Wallet::GetOutputs(TxoState type) const noexcept -> std::vector<UTXO>
{
    return convert(db_.GetOutputs(type));
}

auto Wallet::GetOutputs(const identifier::Nym& owner, TxoState type)
    const noexcept -> std::vector<UTXO>
{
    return convert(db_.GetOutputs(owner, type));
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const Identifier& node,
    TxoState type) const noexcept -> std::vector<UTXO>
{
    return convert(db_.GetOutputs(owner, node, type));
}

auto Wallet::Init() noexcept -> void
{
    enabled_ = true;
    trigger();
}

auto Wallet::pipeline(const zmq::Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::shutdown: {
            shutdown(shutdown_promise_);
        } break;
        case Work::block: {
            // nothing to do until the filter is downloaded
        } break;
        case Work::reorg: {
            process_reorg(in);
            do_work();
        } break;
        case Work::mempool: {
            process_mempool(in);
            do_work();
        } break;
        case Work::nym: {
            OT_ASSERT(1 < body.size());

            accounts_.Add(body.at(1));
            [[fallthrough]];
        }
        case Work::key:
        case Work::filter:
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Wallet::process_mempool(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();
    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    if (auto tx = mempool_.Query(body.at(2).Bytes()); tx) {
        accounts_.Mempool(std::move(tx));
    }
}

auto Wallet::process_reorg(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    if (4 > body.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto parent = block::Position{
        body.at(3).as<block::Height>(),
        api_.Factory().Data(body.at(2).Bytes())};
    accounts_.Reorg(parent);
}

auto Wallet::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (running_->Off()) {
        LogDetail("Shutting down ")(DisplayString(chain_))(" wallet").Flush();
        accounts_.shutdown();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto Wallet::state_machine() noexcept -> bool
{
    if (false == running_.get()) { return false; }

    auto repeat = accounts_.state_machine(enabled_);

    if (enabled_) { repeat |= proposals_.Run(); }

    return repeat;
}

Wallet::~Wallet() { Shutdown().get(); }
}  // namespace opentxs::blockchain::node::implementation

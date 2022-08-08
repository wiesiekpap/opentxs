// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Progress.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <chrono>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
Progress::Imp::Imp(
    const boost::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          JobType::progress,
          alloc,
          {},
          {
              {parent->to_progress_endpoint_, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {parent->to_scan_endpoint_, Direction::Connect},
               }},
          })
    , to_scan_(pipeline_.Internal().ExtraSocket(1))
{
}

auto Progress::Imp::do_process_update(Message&& msg) noexcept -> void
{
    const auto start = Clock::now();
    auto clean = Set<ScanStatus>{get_allocator()};
    auto dirty = Set<block::Position>{get_allocator()};
    decode(parent_.api_, msg, clean, dirty);
    const auto decoded = Clock::now();

    OT_ASSERT(0u == dirty.size());
    OT_ASSERT(0u < clean.size());

    const auto& best = clean.crbegin()->second;
    auto handle = parent_.progress_position_.lock();
    auto& last = *handle;

    if ((false == last.has_value()) || (last.value() != best)) {
        parent_.db_.SubchainSetLastScanned(parent_.db_key_, best);
        last = best;
        notify(best);
        to_scan_.Send(MakeWork(Work::statemachine));
    }

    const auto database = Clock::now();
    parent_.match_cache_.lock()->Forget(best);
    const auto cache = Clock::now();
    log_(OT_PRETTY_CLASS())(name_)(" time to decode update: ")(
        std::chrono::nanoseconds{decoded - start})
        .Flush();
    log_(OT_PRETTY_CLASS())(name_)(" time to update database ")(
        std::chrono::nanoseconds{database - decoded})
        .Flush();
    log_(OT_PRETTY_CLASS())(name_)(" time to update cache: ")(
        std::chrono::nanoseconds{cache - database})
        .Flush();
}

auto Progress::Imp::notify(const block::Position& pos) const noexcept -> void
{
    parent_.ReportScan(pos);
}

auto Progress::Imp::ProcessReorg(
    const Lock& headerOracleLock,
    const block::Position& parent) noexcept -> void
{
    synchronize([&lock = std::as_const(headerOracleLock), &parent, this] {
        sProcessReorg(lock, parent);
    });
}

auto Progress::Imp::sProcessReorg(
    const Lock& headerOracleLock,
    const block::Position& parent) noexcept -> void
{
    auto handle = parent_.progress_position_.lock();
    auto& last = *handle;

    if (last.has_value()) {
        const auto target =
            parent_.ReorgTarget(headerOracleLock, parent, last.value());
        last = target;
        notify(target);
    }
}

auto Progress::Imp::process_do_rescan(Message&& in) noexcept -> void
{
    parent_.progress_position_.lock()->reset();
    const auto& best = parent_.null_position_;
    parent_.db_.SubchainSetLastScanned(parent_.db_key_, best);
    notify(best);
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Progress::Progress(
    const boost::shared_ptr<const SubchainStateData>& parent) noexcept
    : imp_([&] {
        const auto& asio = parent->api_.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, parent, batchID);
    }())
{
    imp_->Init(imp_);
}

auto Progress::ChangeState(const State state, StateSequence reorg) noexcept
    -> bool
{
    return imp_->ChangeState(state, reorg);
}

auto Progress::ProcessReorg(
    const Lock& headerOracleLock,
    const block::Position& parent) noexcept -> void
{
    imp_->ProcessReorg(headerOracleLock, parent);
}

Progress::~Progress()
{
    if (imp_) { imp_->Shutdown(); }
}
}  // namespace opentxs::blockchain::node::wallet

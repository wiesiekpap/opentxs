// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Scan.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <array>
#include <cstddef>
#include <limits>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Actor.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"
#include "util/tuning.hpp"

namespace opentxs::blockchain::node::wallet
{
Scan::Imp::Imp(
    const boost::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          JobType::scan,
          alloc,
          {
              {CString{parent->api_.Endpoints().BlockchainNewFilter()},
               Direction::Connect},
          },
          {
              {parent->to_scan_endpoint_, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {parent->to_process_endpoint_, Direction::Connect},
               }},
          })
    , to_process_(pipeline_.Internal().ExtraSocket(1))
    , last_scanned_(std::nullopt)
    , filter_tip_(std::nullopt)
    , enabled_(false)
{
}

Scan::Imp::~Imp() { tdiag("Scan::Imp::~Imp"); }

auto Scan::Imp::caught_up() const noexcept -> bool
{
    return current() == filter_tip_.value_or(parent_.null_position_);
}

auto Scan::Imp::current() const noexcept -> const block::Position&
{
    if (last_scanned_.has_value()) {

        return last_scanned_.value();
    } else {

        return parent_.null_position_;
    }
}

auto Scan::Imp::do_startup() noexcept -> void
{
    disable_automatic_processing(true);
    const auto& node = parent_.node_;
    const auto& filters = node.FilterOracleInternal();
    last_scanned_ = parent_.db_.SubchainLastScanned(parent_.db_key_);
    filter_tip_ = filters.FilterTip(parent_.filter_type_);

    OT_ASSERT(last_scanned_.has_value());
    OT_ASSERT(filter_tip_.has_value());

    log_(OT_PRETTY_CLASS())(parent_.name_)(" loaded last scanned value of ")(
        last_scanned_.value())(" from database")
        .Flush();
    log_(OT_PRETTY_CLASS())(parent_.name_)(" loaded filter tip value of ")(
        last_scanned_.value())(" from filter oracle")
        .Flush();

    if (last_scanned_.value() > filter_tip_.value()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" last scanned reset to ")(
            filter_tip_.value())
            .Flush();
        last_scanned_ = filter_tip_;
    }

    auto work = MakeWork(Work::update);
    add_last_reorg(work);

    auto clean = Vector<ScanStatus>{get_allocator()};
    clean.emplace_back(ScanState::scan_clean, last_scanned_.value());
    encode(clean, work);

    to_process_.SendDeferred(std::move(work));
}

auto Scan::Imp::ProcessReorg(
    const Lock& headerOracleLock,
    const block::Position& parent) noexcept -> void
{
    synchronize([&lock = std::as_const(headerOracleLock), &parent, this] {
        sProcessReorg(lock, parent);
    });
}
auto Scan::Imp::sProcessReorg(
    const Lock& headerOracleLock,
    const block::Position& parent) noexcept -> void
{
    if (last_scanned_.has_value()) {
        const auto target = parent_.ReorgTarget(
            headerOracleLock, parent, last_scanned_.value());
        log_(OT_PRETTY_CLASS())(parent_.name_)(" last scanned reset to ")(
            target)
            .Flush();
        last_scanned_ = target;
    }

    if (filter_tip_.has_value() && (filter_tip_.value() > parent)) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" filter tip reset to ")(parent)
            .Flush();
        filter_tip_ = parent;
    }
}

auto Scan::Imp::process_do_rescan(Message&& in) noexcept -> void
{
    last_scanned_.reset();
    parent_.match_cache_.lock()->Reset();
    to_process_.Send(std::move(in));
}

auto Scan::Imp::process_filter(Message&& in, block::Position&& tip) noexcept
    -> void
{
    if (tip < this->tip()) {
        log_(OT_PRETTY_CLASS())(name_)(" ignoring stale filter tip ")(tip)
            .Flush();

        return;
    }

    log_(OT_PRETTY_CLASS())(parent_.name_)(" filter tip updated to ")(tip)
        .Flush();
    filter_tip_ = std::move(tip);

    if (auto last = last_reorg(); last.has_value()) {
        in.AddFrame(last.value());
    }

    to_process_.Send(std::move(in));
    do_work();
}

auto Scan::Imp::tip() const noexcept -> const block::Position&
{
    if (filter_tip_.has_value()) {

        return filter_tip_.value();
    } else {

        return parent_.null_position_;
    }
}

auto Scan::Imp::work() noexcept -> int
{
    auto post = ScopeGuard{[&] { Job::work(); }};

    if (false == filter_tip_.has_value()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(
            " scanning not possible until a filter tip value is received ")
            .Flush();

        return SM_off;
    }

    if (false == enabled_) {
        enabled_ = parent_.node_.IsWalletScanEnabled();

        if (false == enabled_) {
            log_(OT_PRETTY_CLASS())(parent_.name_)(
                " waiting to begin scan until cfilter sync is complete")
                .Flush();

            return SM_off;
        } else {
            log_(OT_PRETTY_CLASS())(parent_.name_)(
                " starting scan since cfilter sync is complete")
                .Flush();
        }
    }

    if (caught_up()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(
            " all available filters have been scanned")
            .Flush();

        return SM_off;
    }

    const auto height = current().height_;
    opentxs::blockchain::block::Height rescan{};
    if (auto handle = parent_.progress_position_.lock(); handle->has_value())
        rescan = handle->value().height_;
    else
        rescan = SM_off;

    const auto& threshold = parent_.scan_threshold_;

    if (parent_.scan_dirty_ && ((height - rescan) > threshold)) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(
            " waiting to continue scan until rescan has caught up to block ")(
            height - threshold)(" from current position of ")(rescan)
            .Flush();

        return SM_off;
    }

    auto buf = std::array<std::byte, scan_status_bytes_ * 1000u>{};
    auto alloc = alloc::BoostMonotonic{buf.data(), buf.size()};
    auto clean = Vector<ScanStatus>{&alloc};
    auto dirty = Vector<ScanStatus>{&alloc};
    auto highestTested = current();
    const auto highestClean = parent_.Scan(
        filter_tip_.value(),
        std::numeric_limits<block::Height>::max(),
        highestTested,
        dirty);
    last_scanned_ = std::move(highestTested);
    log_(OT_PRETTY_CLASS())(parent_.name_)(" last scanned updated to ")(
        current())
        .Flush();

    if (auto count = dirty.size(); 0u < count) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" ")(
            count)(" blocks queued for processing ")
            .Flush();
        // TODO: remove Lambda
        to_process_.SendDeferred([&] {
            auto out = MakeWork(Work::update);
            add_last_reorg(out);
            encode(dirty, out);

            return out;
        }());
    }

    if (highestClean.has_value()) {
        clean.emplace_back(ScanState::scan_clean, highestClean.value());
        // TODO: remove Lambda
        to_process_.SendDeferred([&] {
            auto out = MakeWork(Work::update);
            add_last_reorg(out);
            encode(clean, out);

            return out;
        }());
    }

    return !caught_up() ? SM_Scan_fast : SM_Scan_slow;
}

Scan::Scan(const boost::shared_ptr<const SubchainStateData>& parent) noexcept
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

auto Scan::ChangeState(const State state, StateSequence reorg) noexcept -> bool
{
    return imp_->ChangeState(state, reorg);
}

auto Scan::ProcessReorg(
    const Lock& headerOracleLock,
    const block::Position& parent) noexcept -> void
{
    imp_->ProcessReorg(headerOracleLock, parent);
}

Scan::~Scan()
{
    if (imp_) { imp_->Shutdown(); }
}
}  // namespace opentxs::blockchain::node::wallet

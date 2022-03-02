// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Scan.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <array>
#include <chrono>
#include <cstddef>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
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
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
Scan::Imp::Imp(
    const boost::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Actor(
          parent->api_,
          LogTrace(),
          0ms,
          batch,
          alloc,
          {
              {parent->to_children_endpoint_, Direction::Connect},
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
                   {parent->from_children_endpoint_, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {parent->to_progress_endpoint_, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {parent->to_process_endpoint_, Direction::Connect},
               }},
          })
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , to_progress_(pipeline_.Internal().ExtraSocket(1))
    , to_process_(pipeline_.Internal().ExtraSocket(2))
    , parent_p_(parent)
    , parent_(*parent_p_)
    , state_(State::init)
    , last_scanned_(std::nullopt)
    , filter_tip_(std::nullopt)
{
}

auto Scan::Imp::caught_up() const noexcept -> bool
{
    return last_scanned_.value_or(parent_.null_position_) ==
           filter_tip_.value_or(parent_.null_position_);
}

auto Scan::Imp::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (state_) {
        case State::init: {
            state_init(work, std::move(msg));
        } break;
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

auto Scan::Imp::process_filter(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (parent_.chain_ != chain) { return; }

    const auto type = body.at(2).as<filter::Type>();

    if (type != parent_.node_.FilterOracleInternal().DefaultType()) { return; }

    process_filter(block::Position{
        body.at(3).as<block::Height>(),
        parent_.api_.Factory().Data(body.at(4))});
}

auto Scan::Imp::process_filter(block::Position&& tip) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(parent_.name_)(" filter tip updated to ")(
        opentxs::print(tip))
        .Flush();
    filter_tip_ = std::move(tip);
    do_work();
}

auto Scan::Imp::process_reorg(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(2 < body.size());

    const auto parent = block::Position{
        body.at(1).as<block::Height>(),
        parent_.api_.Factory().Data(body.at(2))};
    process_reorg(parent);
}

auto Scan::Imp::process_reorg(const block::Position& parent) noexcept -> void
{
    if (last_scanned_.has_value() && (last_scanned_.value() > parent)) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" last scanned reset to ")(
            opentxs::print(parent))
            .Flush();
        last_scanned_ = parent;
    }

    if (filter_tip_.has_value() && (filter_tip_.value() > parent)) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" filter tip reset to ")(
            opentxs::print(parent))
            .Flush();
        filter_tip_ = parent;
    }
}

auto Scan::Imp::process_startup(Message&& msg) noexcept -> void
{
    state_ = State::normal;
    do_work();
}

auto Scan::Imp::startup() noexcept -> void
{
    last_scanned_ = parent_.db_.SubchainLastScanned(parent_.db_key_);
    const auto& node = parent_.node_;
    const auto& filters = node.FilterOracleInternal();
    filter_tip_ = filters.FilterTip(parent_.filter_type_);

    if (last_scanned_.has_value()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(
            " loaded last scanned value of ")(
            opentxs::print(last_scanned_.value()))(" from database")
            .Flush();
        to_process_.Send([&] {
            auto out = MakeWork(Work::update);
            auto clean = Vector<ScanStatus>{get_allocator()};
            clean.emplace_back(ScanState::scan_clean, last_scanned_.value());
            encode(clean, out);

            return out;
        }());
    } else {
        log_(OT_PRETTY_CLASS())(parent_.name_)(
            " no previous value for last scanned in database")
            .Flush();
    }

    if (filter_tip_.has_value()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" loaded filter value of ")(
            opentxs::print(last_scanned_.value()))(" from filter oracle")
            .Flush();
    } else {
        log_(OT_PRETTY_CLASS())(parent_.name_)(
            " no filter tip value available from oracle")
            .Flush();
    }
}

auto Scan::Imp::state_init(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::shutdown:
        case Work::filter:
        case Work::reorg_begin:
        case Work::statemachine: {
            // NOTE defer processing of non-startup messages
            pipeline_.Push(std::move(msg));
        } break;
        case Work::reorg_end:
        case Work::do_reorg: {
            LogError()(OT_PRETTY_CLASS())("wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        case Work::startup: {
            process_startup(std::move(msg));
        } break;
        case Work::mempool:
        case Work::block:
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::init:
        case Work::key:
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Scan::Imp::state_normal(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::filter: {
            process_filter(std::move(msg));
        } break;
        case Work::reorg_begin: {
            transition_state_reorg(std::move(msg));
        } break;
        case Work::reorg_end:
        case Work::do_reorg: {
            LogError()(OT_PRETTY_CLASS())("wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        case Work::shutdown:
        case Work::mempool:
        case Work::block:
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::startup:
        case Work::init:
        case Work::key:
        case Work::statemachine:
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Scan::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::shutdown:
        case Work::filter:
        case Work::statemachine: {
            // NOTE defer processing of non-reorg messages until after reorg is
            // complete
            pipeline_.Push(std::move(msg));
        } break;
        case Work::reorg_end: {
            transition_state_normal(std::move(msg));
        } break;
        case Work::reorg_begin: {
            LogError()(OT_PRETTY_CLASS())("wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        case Work::do_reorg: {
            process_reorg(std::move(msg));
        } break;
        case Work::mempool:
        case Work::block:
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::startup:
        case Work::init:
        case Work::key:
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Scan::Imp::transition_state_normal(Message&& msg) noexcept -> void
{
    disable_automatic_processing_ = false;
    state_ = State::normal;
    to_parent_.Send(std::move(msg));
    do_work();
}

auto Scan::Imp::transition_state_reorg(Message&& msg) noexcept -> void
{
    disable_automatic_processing_ = true;
    state_ = State::reorg;
    to_process_.Send(std::move(msg));
}

auto Scan::Imp::work() noexcept -> bool
{
    if (false == filter_tip_.has_value()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(
            " scanning not possible until a filter tip value is received ")
            .Flush();

        return false;
    }

    if (caught_up()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(
            " all available filters have been scanned")
            .Flush();

        return false;
    }

    auto buf = std::
        array<std::byte, scan_status_bytes_ * SubchainStateData::scan_batch_>{};
    auto alloc = alloc::BoostMonotonic{buf.data(), buf.size()};
    auto clean = Vector<ScanStatus>{&alloc};
    auto dirty = Vector<ScanStatus>{&alloc};
    auto highestTested = last_scanned_.value_or(parent_.null_position_);
    const auto highestClean = parent_.Scan(
        filter_tip_.value(),
        std::numeric_limits<block::Height>::max(),
        highestTested,
        dirty);
    last_scanned_ = std::move(highestTested);
    log_(OT_PRETTY_CLASS())(parent_.name_)(" last scanned updated to ")(
        opentxs::print(last_scanned_.value_or(parent_.null_position_)))
        .Flush();

    if (auto count = dirty.size(); 0u < count) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" ")(
            count)(" blocks queued for processing ")
            .Flush();
        to_process_.Send([&] {
            auto out = MakeWork(Work::update);
            encode(dirty, out);

            return out;
        }());
    }

    if (highestClean.has_value()) {
        clean.emplace_back(ScanState::scan_clean, highestClean.value());
        to_process_.Send([&] {
            auto out = MakeWork(Work::update);
            encode(clean, out);

            return out;
        }());
    }

    return (false == caught_up());
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
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

Scan::~Scan() = default;
}  // namespace opentxs::blockchain::node::wallet

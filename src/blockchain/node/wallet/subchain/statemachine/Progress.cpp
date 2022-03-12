// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Progress.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <chrono>
#include <iterator>
#include <optional>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
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
Progress::Imp::Imp(
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
          },
          {
              {parent->to_progress_endpoint_, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {parent->from_children_endpoint_, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {parent->to_rescan_endpoint_, Direction::Connect},
               }},
          })
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , to_rescan_(pipeline_.Internal().ExtraSocket(1))
    , parent_p_(parent)
    , parent_(*parent_p_)
    , state_(State::normal)
    , last_reported_(std::nullopt)
{
}

auto Progress::Imp::do_shutdown() noexcept -> void { parent_p_.reset(); }

auto Progress::Imp::notify(const block::Position& pos) const noexcept -> void
{
    parent_.ReportScan(pos);
}

auto Progress::Imp::pipeline(const Work work, Message&& msg) noexcept -> void
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

auto Progress::Imp::process_reorg(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(2 < body.size());

    const auto parent = block::Position{
        body.at(1).as<block::Height>(),
        parent_.api_.Factory().Data(body.at(2))};
    process_reorg(parent);
}

auto Progress::Imp::process_reorg(const block::Position& parent) noexcept
    -> void
{
    if (last_reported_.has_value() && last_reported_.value() > parent) {
        last_reported_ = parent;
        notify(parent);
    }
}

auto Progress::Imp::process_update(Message&& msg) noexcept -> void
{
    auto clean = Set<ScanStatus>{get_allocator()};
    auto dirty = Set<block::Position>{get_allocator()};
    decode(parent_.api_, msg, clean, dirty);

    OT_ASSERT(0u == dirty.size());
    OT_ASSERT(0u < clean.size());

    const auto& best = clean.crbegin()->second;
    auto& last = last_reported_;

    if ((false == last.has_value()) || (last.value() != best)) {
        parent_.db_.SubchainSetLastScanned(parent_.db_key_, best);
        last = best;
        notify(best);
    }
}

auto Progress::Imp::state_normal(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::reorg_begin: {
            transition_state_reorg(std::move(msg));
        } break;
        case Work::reorg_end:
        case Work::do_reorg: {
            LogError()(OT_PRETTY_CLASS())(parent_.name_)(
                " wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        case Work::update: {
            process_update(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::startup:
        case Work::init:
        case Work::key:
        case Work::statemachine:
        default: {
            LogError()(OT_PRETTY_CLASS())(parent_.name_)("unhandled type")
                .Flush();

            OT_FAIL;
        }
    }
}

auto Progress::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::shutdown:
        case Work::update:
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::reorg_end: {
            transition_state_normal(std::move(msg));
        } break;
        case Work::reorg_begin: {
            LogError()(OT_PRETTY_CLASS())(parent_.name_)(
                " wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        case Work::do_reorg: {
            process_reorg(std::move(msg));
        } break;
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::startup:
        case Work::init:
        case Work::key:
        default: {
            LogError()(OT_PRETTY_CLASS())(parent_.name_)("unhandled type")
                .Flush();

            OT_FAIL;
        }
    }
}

auto Progress::Imp::transition_state_normal(Message&& msg) noexcept -> void
{
    disable_automatic_processing_ = false;
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(parent_.name_)(" transitioned to normal state ")
        .Flush();
    to_rescan_.Send(std::move(msg));
    flush_cache();
}

auto Progress::Imp::transition_state_reorg(Message&& msg) noexcept -> void
{
    disable_automatic_processing_ = true;
    state_ = State::reorg;
    log_(OT_PRETTY_CLASS())(parent_.name_)(" transitioned to reorg state ")
        .Flush();
    to_parent_.Send(MakeWork(Work::reorg_begin_ack));
}

auto Progress::Imp::VerifyState(const State state) const noexcept -> void
{
    OT_ASSERT(state == state_);
}

auto Progress::Imp::work() noexcept -> bool
{
    OT_FAIL;  // NOTE not used
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

auto Progress::VerifyState(const State state) const noexcept -> void
{
    imp_->VerifyState(state);
}

Progress::~Progress() = default;
}  // namespace opentxs::blockchain::node::wallet

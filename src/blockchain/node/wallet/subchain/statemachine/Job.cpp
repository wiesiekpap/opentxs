// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"  // IWYU pragma: associated

#include <chrono>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(JobState state) noexcept -> std::string_view
{
    try {
        static const auto map = Map<JobState, std::string_view>{
            {JobState::normal, "normal"},
            {JobState::reorg, "reorg"},
            {JobState::shutdown, "shutdown"},
        };

        return map.at(state);
    } catch (...) {
        LogError()(__FUNCTION__)(": invalid JobState: ")(
            static_cast<OTZMQWorkType>(state))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet::statemachine
{
Job::Job(
    const Log& logger,
    const boost::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    CString&& name,
    allocator_type alloc,
    const network::zeromq::EndpointArgs& subscribe,
    const network::zeromq::EndpointArgs& pull,
    const network::zeromq::EndpointArgs& dealer,
    const Vector<network::zeromq::SocketData>& extra,
    Set<Work>&& neverDrop) noexcept
    : Actor(
          parent->api_,
          logger,
          [&] {
              using namespace std::literals;
              auto out = std::move(name);
              out.append(" job for "sv);
              out.append(parent->name_);

              return out;
          }(),
          0s,
          batch,
          alloc,
          [&] {
              auto out{subscribe};
              out.emplace_back(parent->shutdown_endpoint_, Direction::Connect);

              return out;
          }(),
          pull,
          dealer,
          extra,
          std::move(neverDrop))
    , parent_p_(parent)
    , parent_(*parent_p_)
    , pending_state_(State::normal)
    , state_(State::normal)
    , reorgs_(alloc)
{
    OT_ASSERT(parent_p_);
}

auto Job::ChangeState(const State state, StateSequence reorg) noexcept -> bool
{
    if (auto old = pending_state_.exchange(state); old == state) {

        return true;
    }

    auto lock = lock_for_reorg(name_, reorg_lock_);
    auto output{false};

    switch (state) {
        case State::normal: {
            if (State::reorg != state_) { break; }

            transition_state_normal();
            output = true;
        } break;
        case State::reorg: {
            if (State::shutdown == state_) { break; }

            transition_state_reorg(reorg);
            output = true;
        } break;
        case State::shutdown: {
            if (State::reorg == state_) { break; }

            transition_state_shutdown();
            output = true;
        } break;
        default: {
            OT_FAIL;
        }
    }

    if (false == output) {
        LogError()(OT_PRETTY_CLASS())(name_)(" failed to change state from ")(
            print(state_))(" to ")(print(state))
            .Flush();

        OT_FAIL;
    }

    return output;
}

auto Job::do_shutdown() noexcept -> void {}

auto Job::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::shutdown: {
            shutdown_actor();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Job::process_block(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (parent_.chain_ != chain) { return; }

    process_block(parent_.api_.Factory().Data(body.at(2)));
}

auto Job::process_block(block::pHash&&) noexcept -> void
{
    LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Flush();

    OT_FAIL;
}

auto Job::process_filter(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (parent_.chain_ != chain) { return; }

    const auto type = body.at(2).as<cfilter::Type>();

    if (type != parent_.node_.FilterOracleInternal().DefaultType()) { return; }

    process_filter(block::Position{
        body.at(3).as<block::Height>(),
        parent_.api_.Factory().Data(body.at(4))});
}

auto Job::process_filter(block::Position&& tip) noexcept -> void
{
    LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Flush();

    OT_FAIL;
}

auto Job::process_key(Message&& in) noexcept -> void
{
    LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Flush();

    OT_FAIL;
}

auto Job::process_prepare_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1u < body.size());

    transition_state_reorg(body.at(1).as<StateSequence>());
}

auto Job::process_startup(Message&& msg) noexcept -> void
{
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to normal state ").Flush();
    disable_automatic_processing_ = false;
    flush_cache();
    do_work();
}

auto Job::process_mempool(Message&& in) noexcept -> void
{
    LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Flush();

    OT_FAIL;
}

auto Job::process_update(Message&& msg) noexcept -> void
{
    LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Flush();

    OT_FAIL;
}

auto Job::state_normal(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::shutdown: {
            shutdown_actor();
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
        case Work::prepare_reorg: {
            process_prepare_reorg(std::move(msg));
        } break;
        case Work::update: {
            process_update(std::move(msg));
        } break;
        case Work::init: {
            do_init();
        } break;
        case Work::key: {
            process_key(std::move(msg));
        } break;
        case Work::prepare_shutdown: {
            transition_state_shutdown();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Job::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::prepare_reorg:
        case Work::update:
        case Work::statemachine: {
            log_(OT_PRETTY_CLASS())(name_)(" deferring ")(print(work))(
                " message processing until reorg is complete")
                .Flush();
            defer(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::prepare_shutdown: {
            LogError()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Flush();

            OT_FAIL;
        }
        case Work::key:
        default: {
            LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Job::transition_state_normal() noexcept -> void
{
    disable_automatic_processing_ = false;
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to normal state ").Flush();
    trigger();
}

auto Job::transition_state_reorg(StateSequence id) noexcept -> void
{
    OT_ASSERT(0u < id);

    if (0u == reorgs_.count(id)) {
        reorgs_.emplace(id);
        disable_automatic_processing_ = true;
        state_ = State::reorg;
        log_(OT_PRETTY_CLASS())(name_)(" ready to process reorg ")(id).Flush();
    } else {
        log_(OT_PRETTY_CLASS())(name_)(" reorg ")(id)(" already handled")
            .Flush();
    }
}

auto Job::transition_state_shutdown() noexcept -> void
{
    state_ = State::shutdown;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to shutdown state ").Flush();
}

Job::~Job() = default;
}  // namespace opentxs::blockchain::node::wallet::statemachine

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/node/p2p/Requestor.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <string_view>
#include <utility>

#include "internal/api/network/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/p2p/Acknowledgement.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/P2PBlockchainChainState.pb.h"
#include "util/Work.hpp"
#include "util/tuning.hpp"

namespace opentxs::blockchain::node::p2p
{
Requestor::Imp::Imp(
    const api::Session& api,
    const network::zeromq::BatchID batch,
    const Type chain,
    const std::string_view toParent,
    allocator_type alloc) noexcept
    : Actor(
          api,
          LogTrace(),
          std::string(print(chain)) + " p2p requestor",
          batch,
          alloc,
          {
              {CString{api.Endpoints().Shutdown()}, Direction::Connect},
          },
          {},
          {
              {CString{api.Network().Blockchain().Internal().SyncEndpoint()},
               Direction::Connect},
          },
          {
              {SocketType::Pair,
               {
                   {CString{toParent}, Direction::Bind},
               }},
          })
    , api_(api)
    , chain_(chain)
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , state_(State::init)
    , last_remote_position_()
    , begin_sync_()
    , last_request_(std::nullopt)
    , remote_position_(blank(api_))
    , local_position_(blank(api_))
    , queue_position_(blank(api_))
    , queued_bytes_()
    , processed_bytes_()
    , queue_()
    , received_first_ack_(false)
    , processing_(false)
{
}

auto Requestor::Imp::add_to_queue(
    const network::p2p::Data& data,
    Message&& msg) noexcept -> void
{
    const auto& blocks = data.Blocks();

    if (0u == blocks.size()) { return; }

    const auto bytes = msg.Total();
    log_(OT_PRETTY_CLASS())("buffering ")(bytes)(" bytes of ")(print(chain_))(
        " sync data for ")(print(chain_))(" blocks ")(blocks.front().Height())(
        " to ")(blocks.back().Height())
        .Flush();
    queued_bytes_ += bytes;
    processed_bytes_ += bytes;
    queue_.emplace(std::move(msg));
    update_queue_position(data);
}

auto Requestor::Imp::blank(const api::Session& api) noexcept
    -> const block::Position&
{
    static const auto output = block::Position{-1, block::Hash{}};

    return output;
}

auto Requestor::Imp::blank() const noexcept -> const block::Position&
{
    return blank(api_);
}

auto Requestor::Imp::check_remote_position() noexcept -> void
{
    const auto interval = Clock::now() - last_remote_position_;

    if (interval > remote_position_timeout_) {
        static constexpr auto timeout =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                remote_position_timeout_);
        log_(OT_PRETTY_CLASS())("last remote position considered stale after ")(
            timeout)
            .Flush();
        remote_position_ = blank();
    } else {
        const auto remaining =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                remote_position_timeout_ - interval);
        log_(OT_PRETTY_CLASS())("last remote position still valid for ")(
            remaining)
            .Flush();
    }
}

auto Requestor::Imp::do_common() noexcept -> void
{
    if (need_sync()) { request(next_position()); }

    if (processing_) { return; }

    if (0 < queue_.size()) {
        auto& msg = queue_.front();
        const auto bytes = msg.Total();
        to_parent_.Send(std::move(msg));
        processing_ = true;
        queued_bytes_ -= bytes;
        queue_.pop();
        update_queue_position();
    }
}

auto Requestor::Imp::do_run() noexcept -> void { do_common(); }

auto Requestor::Imp::do_shutdown() noexcept -> void
{
    state_ = State::quitting;
}

auto Requestor::Imp::do_startup() noexcept -> void
{
    register_chain();
    do_work();
}

auto Requestor::Imp::do_sync() noexcept -> void
{
    do_common();
    transition_state_run();
}

auto Requestor::Imp::have_pending_request() noexcept -> bool
{
    if (last_request_.has_value()) {
        const auto& last = last_request_.value();
        const auto interval = Clock::now() - last;

        if (interval > request_timeout_) {
            static constexpr auto timeout =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    request_timeout_);
            log_(OT_PRETTY_CLASS())("pending request has timed out after ")(
                timeout)
                .Flush();
            last_request_.reset();

            return false;
        } else {
            const auto remaining =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    request_timeout_ - interval);
            log_(OT_PRETTY_CLASS())("waiting ")(
                remaining)(" before making a new request")
                .Flush();

            return true;
        }
    } else {

        return false;
    }
}

auto Requestor::Imp::need_sync() noexcept -> bool
{
    if (blank() == remote_position_) { return true; }

    if (queue_position_ == remote_position_) {
        log_(OT_PRETTY_CLASS())(print(chain_))(" data already queued").Flush();

        return false;
    }

    check_remote_position();

    if (local_position_ == remote_position_) {
        log_(OT_PRETTY_CLASS())(print(chain_))(" is up to date").Flush();

        return false;
    }

    if (queued_bytes_ >= limit_) {
        log_(OT_PRETTY_CLASS())(print(chain_))(" buffer is full with ")(
            queued_bytes_)(" bytes ")
            .Flush();

        return false;
    }

    return true;
}

auto Requestor::Imp::next_position() const noexcept -> const block::Position&
{
    return (blank() == queue_position_) ? local_position_ : queue_position_;
}

auto Requestor::Imp::to_str(Work w) const noexcept -> std::string
{
    return std::string(print(w));
}

auto Requestor::Imp::pipeline(const Work work, Message&& msg) noexcept -> void
{
    tadiag("pipeline ", std::string{print(work)});

    switch (state_) {
        case State::init: {
            state_init(work, std::move(msg));
        } break;
        case State::sync: {
            state_sync(work, std::move(msg));
        } break;
        case State::run: {
            state_run(work, std::move(msg));
        } break;
        case State::quitting: {
            // ignore.
            break;
        }
        default: {
            OT_FAIL;
        }
    }
}

auto Requestor::Imp::process_push_tx(Message&& in) noexcept -> void
{
    pipeline_.Internal().SendFromThread(std::move(in));
}

auto Requestor::Imp::process_sync_ack(Message&& in) noexcept -> void
{
    received_first_ack_ = true;
    const auto base = api_.Factory().BlockchainSyncMessage(in);
    const auto& ack = base->asAcknowledgement();
    update_remote_position(ack.State(chain_));
    log_(OT_PRETTY_CLASS())("best chain tip for ")(print(chain_))(
        " according to sync peer is ")(remote_position_)
        .Flush();
}

auto Requestor::Imp::process_sync_processed(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    local_position_ = {
        body.at(1).as<block::Height>(), block::Hash{body.at(2).Bytes()}};
    processing_ = false;
}

auto Requestor::Imp::process_sync_push(Message&& in) noexcept -> void
{
    const auto base = api_.Factory().BlockchainSyncMessage(in);
    const auto& data = base->asData();
    update_remote_position(data);
    add_to_queue(data, std::move(in));
}

auto Requestor::Imp::process_sync_reply(Message&& in) noexcept -> void
{
    const auto base = api_.Factory().BlockchainSyncMessage(in);
    const auto& data = base->asData();
    update_remote_position(data);
    last_request_.reset();
    add_to_queue(data, std::move(in));
}

auto Requestor::Imp::register_chain() noexcept -> void
{
    log_(OT_PRETTY_CLASS())("registering ")(print(chain_))(
        " with high level api")
        .Flush();
    pipeline_.Internal().SendFromThread([&] {
        auto out = MakeWork(Work::Register);
        out.AddFrame(chain_);

        return out;
    }());
}

auto Requestor::Imp::request(const block::Position& position) noexcept -> void
{
    if (blank() == position) { return; }

    if (have_pending_request()) { return; }

    if (false == received_first_ack_) {
        log_(OT_PRETTY_CLASS())("waiting to request ")(print(chain_))(
            " sync data until a data provider is available ")
            .Flush();

        return;
    }

    log_(OT_PRETTY_CLASS())("requesting ")(print(chain_))(
        " sync data starting from block ")(position.height_)
        .Flush();
    // TODO remove Lambda
    pipeline_.Internal().SendFromThread([&] {
        auto msg = MakeWork(Work::Request);
        msg.AddFrame(chain_);
        msg.Internal().AddFrame([&] {
            auto proto = proto::P2PBlockchainChainState{};
            const auto state = network::p2p::State{chain_, position};
            state.Serialize(proto);

            return proto;
        }());

        return msg;
    }());
    last_request_ = Clock::now();
}

auto Requestor::Imp::state_init(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::Shutdown: {
            shutdown_actor();
        } break;
        case Work::PushTransaction: {
            tdiag("... defer(&&)");
            defer(std::move(msg));
        } break;
        case Work::Register: {
            transition_state_sync();
        } break;
        case Work::Processed: {
            // TODO change work type for peer header received messages
        } break;
        case Work::Init: {
            do_init();
        } break;
        case Work::StateMachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Requestor::Imp::state_run(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::Shutdown: {
            shutdown_actor();
        } break;
        case Work::SyncAck: {
            process_sync_ack(std::move(msg));
        } break;
        case Work::SyncReply: {
            process_sync_reply(std::move(msg));
        } break;
        case Work::SyncPush: {
            process_sync_push(std::move(msg));
        } break;
        case Work::PushTransaction: {
            process_push_tx(std::move(msg));
        } break;
        case Work::Register: {
            // NOTE duplicate Register messages are possible so they should be
            // ignored
        } break;
        case Work::Processed: {
            process_sync_processed(std::move(msg));
        } break;
        case Work::Init: {
            do_init();
        } break;
        case Work::StateMachine: {
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }

    do_work();
}

auto Requestor::Imp::state_sync(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::Shutdown: {
            shutdown_actor();
        } break;
        case Work::SyncAck: {
            process_sync_ack(std::move(msg));
        } break;
        case Work::SyncReply: {
            process_sync_reply(std::move(msg));
        } break;
        case Work::SyncPush: {
            update_remote_position(msg);
        } break;
        case Work::PushTransaction: {
            process_push_tx(std::move(msg));
        } break;
        case Work::Register: {
            // NOTE duplicate Register messages are possible so they should be
            // ignored
        } break;
        case Work::Processed: {
            process_sync_processed(std::move(msg));
        } break;
        case Work::Init: {
            do_init();
        } break;
        case Work::StateMachine: {
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }

    do_work();
}

auto Requestor::Imp::transition_state_run() noexcept -> void
{
    const auto checkpoint = params::Chains().at(chain_).checkpoint_.height_;

    if (remote_position_.height_ < checkpoint ||
        local_position_ != remote_position_ || !queue_.empty() ||
        last_request_.has_value() || processing_) {
        return;
    }

    const auto interval = std::chrono::duration_cast<std::chrono::nanoseconds>(
        Clock::now() - begin_sync_);
    const auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(interval).count();
    const auto kb = processed_bytes_ / 1024u;
    const auto mb = kb / 1024u;
    LogConsole()(print(chain_))(" sync complete. Processed ")(mb)(" MiB in ")(
        interval)(" (")(kb / seconds)(" KiB/sec)")
        .Flush();
    state_ = State::run;
}

auto Requestor::Imp::transition_state_sync() noexcept -> void
{
    begin_sync_ = Clock::now();
    state_ = State::sync;
    flush_cache();
}

auto Requestor::Imp::update_queue_position() noexcept -> void
{
    if (0 == queue_.size()) {
        queue_position_ = blank();

        return;
    }

    if (blank() == queue_position_) {
        const auto base = api_.Factory().BlockchainSyncMessage(queue_.back());
        update_queue_position(base->asData());
    }
}

auto Requestor::Imp::update_queue_position(
    const network::p2p::Data& data) noexcept -> void
{
    const auto& blocks = data.Blocks();

    if (0u == blocks.size()) { return; }

    queue_position_ = data.LastPosition(api_);
}

auto Requestor::Imp::update_remote_position(const Message& msg) noexcept -> void
{
    const auto base = api_.Factory().BlockchainSyncMessage(msg);
    update_remote_position(base->asData());
}

auto Requestor::Imp::update_remote_position(
    const network::p2p::Data& data) noexcept -> void
{
    update_remote_position(data.State());
}

auto Requestor::Imp::update_remote_position(
    const network::p2p::State& state) noexcept -> void
{
    remote_position_ = state.Position();
    last_remote_position_ = Clock::now();
}

auto Requestor::Imp::work() noexcept -> int
{
    switch (state_) {
        case State::init: {
            return SM_Requestor_fast;
        }
        case State::sync: {
            do_sync();
            return SM_Requestor_fast;
        }
        case State::run: {
            do_run();
            return SM_Requestor_fast;
        }
        case State::quitting: {
            return SM_off;
        }
        default: {
            OT_FAIL;
        }
    }
}

Requestor::Imp::~Imp() { signal_shutdown(); }
}  // namespace opentxs::blockchain::node::p2p

namespace opentxs::blockchain::node::p2p
{
Requestor::Requestor(
    const api::Session& api,
    const Type chain,
    const std::string_view toParent) noexcept
    : imp_([&] {
        const auto& asio = api.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)},
            api,
            batchID,
            chain,
            toParent);
    }())
{
    imp_->Init(imp_);
}

Requestor::~Requestor() { imp_->Shutdown(); }
}  // namespace opentxs::blockchain::node::p2p

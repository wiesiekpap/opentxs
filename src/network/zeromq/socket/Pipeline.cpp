// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "network/zeromq/socket/Pipeline.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Thread.hpp"
#include "internal/network/zeromq/message/Message.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Gatekeeper.hpp"

namespace opentxs::factory
{
auto Pipeline(
    const api::Session& api,
    const network::zeromq::Context& context,
    std::function<void(network::zeromq::Message&&)> callback)
    -> opentxs::network::zeromq::Pipeline
{
    return std::make_unique<network::zeromq::implementation::Pipeline>(
               api, context, callback)
        .release();
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq
{
auto swap(Pipeline& lhs, Pipeline& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Pipeline::Pipeline(
    const api::Session& api,
    const zeromq::Context& context,
    std::function<void(zeromq::Message&&)> callback) noexcept
    : Pipeline(
          api,
          context,
          std::move(callback),
          MakeArbitraryInproc(),
          MakeArbitraryInproc())
{
}

Pipeline::Pipeline(
    const api::Session& api,
    const zeromq::Context& context,
    std::function<void(zeromq::Message&&)> callback,
    const UnallocatedCString internalEndpoint,
    const UnallocatedCString outgoingEndpoint) noexcept
    : context_(context)
    , gate_()
    , shutdown_(false)
    , batch_([&]() -> auto& {
        auto& batch = context_.Internal().MakeBatch({
            socket::Type::Subscribe,  // NOTE sub_
            socket::Type::Pull,       // NOTE pull_
            socket::Type::Pair,       // NOTE outgoing_
            socket::Type::Dealer,     // NOTE dealer_
            socket::Type::Pair,       // NOTE internal_
        });
        batch.listen_callbacks_.emplace_back(ListenCallback::Factory(callback));

        return batch;
    }())
    , sub_([&]() -> auto& {
        auto& socket = batch_.sockets_.at(0);
        auto rc = socket.ClearSubscriptions();

        OT_ASSERT(rc);

        rc = socket.SetIncomingHWM(0);

        OT_ASSERT(rc);

        return socket;
    }())
    , pull_([&]() -> auto& {
        auto& socket = batch_.sockets_.at(1);
        const auto rc = socket.SetIncomingHWM(0);

        OT_ASSERT(rc);

        return socket;
    }())
    , outgoing_([&]() -> auto& {
        auto& socket = batch_.sockets_.at(2);
        const auto rc = socket.Bind(outgoingEndpoint.c_str());

        OT_ASSERT(rc);

        return socket;
    }())
    , dealer_([&]() -> auto& {
        auto& socket = batch_.sockets_.at(3);
        auto rc = socket.SetIncomingHWM(0);

        OT_ASSERT(rc);

        rc = socket.SetOutgoingHWM(0);

        OT_ASSERT(rc);

        return socket;
    }())
    , internal_([&]() -> auto& {
        auto& socket = batch_.sockets_.at(4);
        const auto rc = socket.Bind(internalEndpoint.c_str());

        OT_ASSERT(rc);

        return socket;
    }())
    , to_dealer_([&] {
        auto socket = factory::ZMQSocket(context_, socket::Type::Pair);
        auto rc = socket.Connect(outgoingEndpoint.c_str());

        OT_ASSERT(rc);

        rc = socket.SetOutgoingHWM(0);

        OT_ASSERT(rc);

        return socket;
    }())
    , to_internal_([&] {
        auto socket = factory::ZMQSocket(context_, socket::Type::Pair);
        auto rc = socket.Connect(internalEndpoint.c_str());

        OT_ASSERT(rc);

        rc = socket.SetOutgoingHWM(0);

        OT_ASSERT(rc);

        return socket;
    }())
    , thread_(context_.Internal().Start(
          batch_.id_,
          {
              {outgoing_.ID(),
               &outgoing_,
               [id = outgoing_.ID(), socket = &dealer_](auto&& m) {
                   socket->Send(std::move(m));
               }},
              {internal_.ID(),
               &internal_,
               [id = internal_.ID(),
                &cb = batch_.listen_callbacks_.at(0).get()](auto&& m) {
                   m.Internal().Prepend(id);
                   cb.Process(std::move(m));
               }},
              {dealer_.ID(),
               &dealer_,
               [id = dealer_.ID(),
                &cb = batch_.listen_callbacks_.at(0).get()](auto&& m) {
                   m.Internal().Prepend(id);
                   cb.Process(std::move(m));
               }},
              {pull_.ID(),
               &pull_,
               [id = pull_.ID(),
                &cb = batch_.listen_callbacks_.at(0).get()](auto&& m) {
                   m.Internal().Prepend(id);
                   cb.Process(std::move(m));
               }},
              {sub_.ID(),
               &sub_,
               [id = sub_.ID(),
                &cb = batch_.listen_callbacks_.at(0).get()](auto&& m) {
                   m.Internal().Prepend(id);
                   cb.Process(std::move(m));
               }},
          }))
{
    OT_ASSERT(nullptr != thread_);
}

auto Pipeline::bind(
    SocketID id,
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept
    -> std::pair<bool, std::future<bool>>
{
    const auto done = gate_.get();

    if (done) {
        auto null = std::promise<bool>{};
        null.set_value(false);

        return std::make_pair(false, null.get_future());
    }

    return thread_->Modify(
        id,
        [ep = CString{endpoint},
         extra = std::move(notify),
         &cb = batch_.listen_callbacks_.at(0).get()](auto& socket) {
            const auto value = socket.Bind(ep.c_str());

            if (extra) { cb.Process(extra(value)); }
        });
}

auto Pipeline::BindSubscriber(
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept -> bool
{
    return bind(sub_.ID(), endpoint, std::move(notify)).first;
}

auto Pipeline::Close() const noexcept -> bool
{
    gate_.shutdown();

    if (auto sent = shutdown_.exchange(true); false == sent) {
        batch_.ClearCallbacks();
    }

    return true;
}

auto Pipeline::connect(
    SocketID id,
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept
    -> std::pair<bool, std::future<bool>>
{
    const auto done = gate_.get();

    if (done) {
        auto null = std::promise<bool>{};
        null.set_value(false);

        return std::make_pair(false, null.get_future());
    }

    return thread_->Modify(
        id,
        [ep = CString{endpoint},
         extra = std::move(notify),
         &cb = batch_.listen_callbacks_.at(0).get()](auto& socket) {
            const auto value = socket.Connect(ep.c_str());

            if (extra) { cb.Process(extra(value)); }
        });
}

auto Pipeline::ConnectDealer(
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept -> bool
{
    return connect(dealer_.ID(), endpoint, std::move(notify)).first;
}

auto Pipeline::ConnectionIDDealer() const noexcept -> std::size_t
{
    return dealer_.ID();
}

auto Pipeline::ConnectionIDInternal() const noexcept -> std::size_t
{
    return internal_.ID();
}

auto Pipeline::ConnectionIDPull() const noexcept -> std::size_t
{
    return pull_.ID();
}

auto Pipeline::ConnectionIDSubscribe() const noexcept -> std::size_t
{
    return sub_.ID();
}

auto Pipeline::PullFrom(const std::string_view endpoint) const noexcept -> bool
{
    return connect(pull_.ID(), endpoint).first;
}

auto Pipeline::Push(zeromq::Message&& msg) const noexcept -> bool
{
    const auto done = gate_.get();

    if (done) { return false; }

    to_internal_.modify_detach([data = std::move(msg)](auto& socket) mutable {
        socket.Send(std::move(data));
    });

    return true;
}

auto Pipeline::Send(zeromq::Message&& msg) const noexcept -> bool
{
    const auto done = gate_.get();

    if (done) { return false; }

    to_dealer_.modify_detach([data = std::move(msg)](auto& socket) mutable {
        socket.Send(std::move(data));
    });

    return true;
}

auto Pipeline::SubscribeTo(const std::string_view endpoint) const noexcept
    -> bool
{
    return connect(sub_.ID(), endpoint).first;
}

Pipeline::~Pipeline()
{
    Close();
    context_.Internal().Stop(batch_.id_);
}
}  // namespace opentxs::network::zeromq::implementation

namespace opentxs::network::zeromq
{
Pipeline::Pipeline(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Pipeline::Pipeline(Pipeline&& rhs) noexcept
    : Pipeline(std::make_unique<Imp>().release())
{
    swap(rhs);
}

auto Pipeline::operator=(Pipeline&& rhs) noexcept -> Pipeline&
{
    swap(rhs);

    return *this;
}

auto Pipeline::BindSubscriber(
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept -> bool
{
    return imp_->BindSubscriber(endpoint, std::move(notify));
}

auto Pipeline::Close() const noexcept -> bool { return imp_->Close(); }

auto Pipeline::ConnectDealer(
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept -> bool
{
    return imp_->ConnectDealer(endpoint, std::move(notify));
}

auto Pipeline::ConnectionIDDealer() const noexcept -> std::size_t
{
    return imp_->ConnectionIDDealer();
}

auto Pipeline::ConnectionIDInternal() const noexcept -> std::size_t
{
    return imp_->ConnectionIDInternal();
}

auto Pipeline::ConnectionIDPull() const noexcept -> std::size_t
{
    return imp_->ConnectionIDPull();
}

auto Pipeline::ConnectionIDSubscribe() const noexcept -> std::size_t
{
    return imp_->ConnectionIDSubscribe();
}

auto Pipeline::Internal() const noexcept -> const internal::Pipeline&
{
    return *imp_;
}

auto Pipeline::Internal() noexcept -> internal::Pipeline& { return *imp_; }

auto Pipeline::PullFrom(const std::string_view endpoint) const noexcept -> bool
{
    return imp_->PullFrom(endpoint);
}

auto Pipeline::Push(Message&& msg) const noexcept -> bool
{
    return imp_->Push(std::move(msg));
}

auto Pipeline::Send(Message&& msg) const noexcept -> bool
{
    return imp_->Send(std::move(msg));
}

auto Pipeline::SubscribeTo(const std::string_view endpoint) const noexcept
    -> bool
{
    return imp_->SubscribeTo(endpoint);
}

auto Pipeline::swap(Pipeline& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
}

Pipeline::~Pipeline()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq

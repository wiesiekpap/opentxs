// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "network/zeromq/socket/Socket.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>

#include "opentxs/Bytes.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/Message.hpp"
#include "util/Random.hpp"

#define INPROC_PREFIX "inproc://opentxs/"

#define OT_METHOD "opentxs::network::Socket::"

namespace opentxs::network::zeromq::socket::implementation
{
const std::map<SocketType, int> Socket::types_{
    {SocketType::Request, ZMQ_REQ},
    {SocketType::Reply, ZMQ_REP},
    {SocketType::Publish, ZMQ_PUB},
    {SocketType::Subscribe, ZMQ_SUB},
    {SocketType::Pull, ZMQ_PULL},
    {SocketType::Push, ZMQ_PUSH},
    {SocketType::Pair, ZMQ_PAIR},
    {SocketType::Dealer, ZMQ_DEALER},
    {SocketType::Router, ZMQ_ROUTER},
};

Socket::Socket(
    const zeromq::Context& context,
    const SocketType type,
    const Socket::Direction direction) noexcept
    : context_(context)
    , direction_(direction)
    , socket_(zmq_socket(context, types_.at(type)))
    , linger_(0)
    , send_timeout_(0)
    , receive_timeout_(-1)
    , endpoint_lock_()
    , endpoints_()
    , running_(Flag::Factory(true))
    , endpoint_queue_()
    , type_(type)
{
    if (nullptr == socket_) {
        std::cerr << OT_METHOD << __func__ << ": " << zmq_strerror(zmq_errno())
                  << std::endl;
    }

    OT_ASSERT(nullptr != socket_);
}

Socket::operator void*() const noexcept { return socket_; }

void Socket::add_endpoint(const std::string& endpoint) const noexcept
{
    Lock lock(endpoint_lock_);
    endpoints_.emplace(endpoint);
}

auto Socket::apply_socket(SocketCallback&& cb) const noexcept -> bool
{
    Lock lock(lock_);

    return cb(lock);
}

auto Socket::apply_timeouts(const Lock& lock) const noexcept -> bool
{
    OT_ASSERT(nullptr != socket_)
    OT_ASSERT(verify_lock(lock))

    auto set = zmq_setsockopt(socket_, ZMQ_LINGER, &linger_, sizeof(linger_));

    if (0 != set) {
        std::cerr << "Failed to set ZMQ_LINGER\n";
        std::cerr << zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    set = zmq_setsockopt(
        socket_, ZMQ_SNDTIMEO, &send_timeout_, sizeof(send_timeout_));

    if (0 != set) {
        std::cerr << "Failed to set ZMQ_SNDTIMEO\n";
        std::cerr << zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    set = zmq_setsockopt(
        socket_, ZMQ_RCVTIMEO, &receive_timeout_, sizeof(receive_timeout_));

    if (0 != set) {
        std::cerr << "Failed to set ZMQ_RCVTIMEO\n";
        std::cerr << zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Socket::bind(const Lock& lock, const std::string& endpoint) const noexcept
    -> bool
{
    if (false == apply_timeouts(lock)) { return false; }

    const auto output = (0 == zmq_bind(socket_, endpoint.c_str()));

    if (output) {
        add_endpoint(endpoint);
    } else {
        socket_ = nullptr;
        std::cerr << OT_METHOD << __func__ << ": " << zmq_strerror(zmq_errno())
                  << std::endl;
    }

    return output;
}

auto Socket::connect(const Lock& lock, const std::string& endpoint)
    const noexcept -> bool
{
    if (false == apply_timeouts(lock)) { return false; }

    const auto output = (0 == zmq_connect(socket_, endpoint.c_str()));

    if (output) {
        add_endpoint(endpoint);
    } else {
        socket_ = nullptr;
        std::cerr << OT_METHOD << __func__ << ": " << zmq_strerror(zmq_errno())
                  << std::endl;
    }

    return output;
}

auto Socket::Close() const noexcept -> bool
{
    running_->Off();
    Lock lock(lock_);

    if (nullptr == socket_) { return false; }

    const_cast<Socket*>(this)->shutdown(lock);

    return true;
}

auto Socket::receive_message(
    const Lock& lock,
    void* socket,
    zeromq::Message& message) noexcept -> bool
{
    bool receiving{true};

    while (receiving) {
        auto& frame = message.AddFrame();
        const bool received = (-1 != zmq_msg_recv(frame, socket, ZMQ_DONTWAIT));

        if (false == received) {
            auto zerr = zmq_errno();
            if (EAGAIN == zerr) {
                std::cerr << OT_METHOD << __func__
                          << ": zmq_msg_recv returns EAGAIN. This should never "
                             "happen."
                          << std::endl;
            } else {
                std::cerr << OT_METHOD << __func__
                          << ": Receive error: " << zmq_strerror(zerr)
                          << std::endl;
            }

            return false;
        }

        int option{0};
        std::size_t optionBytes{sizeof(option)};

        const bool haveOption =
            (-1 != zmq_getsockopt(socket, ZMQ_RCVMORE, &option, &optionBytes));

        if (false == haveOption) {
            std::cerr << OT_METHOD << __func__
                      << ": Failed to check socket options error:\n"
                      << zmq_strerror(zmq_errno()) << std::endl;

            return false;
        }

        OT_ASSERT(optionBytes == sizeof(option))

        if (1 != option) { receiving = false; }
    }

    return true;
}

auto Socket::send_message(
    const Lock& lock,
    void* socket,
    Message& message) noexcept -> bool
{
    bool sent{true};
    const auto parts = message.size();
    std::size_t counter{0};

    for (auto& frame : message) {
        int flags{0};

        if (++counter < parts) { flags = ZMQ_SNDMORE; }

        sent |= (-1 != zmq_msg_send(frame, socket, flags));
    }

    if (false == sent) {
        std::cerr << OT_METHOD << __func__
                  << ": Send error: " << zmq_strerror(zmq_errno()) << '\n';
    }

    return sent;
}

auto Socket::send_message(const Lock& lock, Message& message) const noexcept
    -> bool
{
    return send_message(lock, socket_, message);
}

auto Socket::random_inproc_endpoint() noexcept -> std::string
{
    auto one = std::uint64_t{};
    auto two = std::uint64_t{};
    random_bytes_non_crypto(preallocated(sizeof(one), &one), sizeof(one));
    random_bytes_non_crypto(preallocated(sizeof(two), &two), sizeof(two));

    return std::string(INPROC_PREFIX) + std::to_string(one) +
           std::to_string(two);
}

auto Socket::receive_message(const Lock& lock, Message& message) const noexcept
    -> bool
{
    return receive_message(lock, socket_, message);
}

auto Socket::set_socks_proxy(const std::string& proxy) const noexcept -> bool
{
    OT_ASSERT(nullptr != socket_);

    SocketCallback cb{[&](const Lock&) -> bool {
        const auto set = zmq_setsockopt(
            socket_, ZMQ_SOCKS_PROXY, proxy.data(), proxy.size());

        return (0 == set);
    }};

    return apply_socket(std::move(cb));
}

auto Socket::SetTimeouts(
    const std::chrono::milliseconds& linger,
    const std::chrono::milliseconds& send,
    const std::chrono::milliseconds& receive) const noexcept -> bool
{
    OT_ASSERT(nullptr != socket_);

    linger_.store(static_cast<int>(linger.count()));
    send_timeout_.store(static_cast<int>(send.count()));
    receive_timeout_.store(static_cast<int>(receive.count()));
    SocketCallback cb{
        [&](const Lock& lock) -> bool { return apply_timeouts(lock); }};

    return apply_socket(std::move(cb));
}

void Socket::shutdown(const Lock& lock) noexcept
{
    if (nullptr == socket_) { return; }

    for (const auto& endpoint : endpoints_) {
        if (Socket::Direction::Connect == direction_) {
            zmq_disconnect(socket_, endpoint.c_str());
        } else {
            zmq_unbind(socket_, endpoint.c_str());
        }
    }

    endpoints_.clear();

    if (0 == zmq_close(socket_)) { socket_ = nullptr; }
}

auto Socket::Start(const std::string& endpoint) const noexcept -> bool
{
    SocketCallback cb{
        [&](const Lock& lock) -> bool { return start(lock, endpoint); }};

    return apply_socket(std::move(cb));
}

auto Socket::StartAsync(const std::string& endpoint) const noexcept -> void
{
    Lock lock{endpoint_queue_.lock_};
    endpoint_queue_.queue_.push(endpoint);
}

auto Socket::start(const Lock& lock, const std::string& endpoint) const noexcept
    -> bool
{
    if (Socket::Direction::Connect == direction_) {

        return connect(lock, endpoint);
    } else {

        return bind(lock, endpoint);
    }
}

Socket::~Socket()
{
    if (nullptr != socket_) {
        zmq_close(socket_);
        socket_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq::socket::implementation

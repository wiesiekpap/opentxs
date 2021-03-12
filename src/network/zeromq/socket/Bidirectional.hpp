// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <zmq.h>
#include <memory>
#include <mutex>
#include <string>

#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Sender.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
template <typename InterfaceType, typename MessageType = zeromq::Message>
class Bidirectional
    : virtual public Receiver<InterfaceType, MessageType>,
      virtual public Sender<InterfaceType, Receiver<InterfaceType, MessageType>>
{
protected:
    Bidirectional(
        const zeromq::Context& context,
        const bool startThread) noexcept;

    void init() noexcept override;
    void shutdown(const Lock& lock) noexcept final;

    ~Bidirectional() override = default;

private:
    using RawSocket = std::unique_ptr<void, decltype(&::zmq_close)>;

    const bool bidirectional_start_thread_;
    const std::string endpoint_;
    RawSocket push_socket_;
    RawSocket pull_socket_;
    mutable int linger_;
    mutable int send_timeout_;
    mutable int receive_timeout_;
    mutable std::mutex send_lock_;

    auto apply_timeouts(void* socket, std::mutex& socket_mutex) const noexcept
        -> bool;
    auto bind(
        void* socket,
        std::mutex& socket_mutex,
        const std::string& endpoint) const noexcept -> bool;
    auto connect(
        void* socket,
        std::mutex& socket_mutex,
        const std::string& endpoint) const noexcept -> bool;

    auto process_pull_socket(const Lock& lock) noexcept -> bool;
    auto process_receiver_socket(const Lock& lock) noexcept -> bool;
    auto send(zeromq::Message& message) const noexcept -> bool final;
    auto send(const Lock& lock, zeromq::Message& message) noexcept -> bool;
    void thread() noexcept final;

    Bidirectional() = delete;
    Bidirectional(const Bidirectional&) = delete;
    Bidirectional(Bidirectional&&) = delete;
    auto operator=(const Bidirectional&) -> Bidirectional& = delete;
    auto operator=(Bidirectional&&) -> Bidirectional& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation

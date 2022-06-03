// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "network/zeromq/socket/Raw.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>

#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"

namespace opentxs
{
auto print(const network::zeromq::socket::Type type) noexcept -> const char*
{
    using Type = network::zeromq::socket::Type;

    static const auto map = robin_hood::unordered_flat_map<Type, const char*>{
        {Type::Request, "ZMQ_REQ"},
        {Type::Reply, "ZMQ_REP"},
        {Type::Publish, "ZMQ_PUB"},
        {Type::Subscribe, "ZMQ_SUB"},
        {Type::Pull, "ZMQ_PULL"},
        {Type::Push, "ZMQ_PUSH"},
        {Type::Pair, "ZMQ_PAIR"},
        {Type::Dealer, "ZMQ_DEALER"},
        {Type::Router, "ZMQ_ROUTER"},
    };

    try {

        return map.at(type);
    } catch (...) {

        return "";
    }
}

auto to_native(const network::zeromq::socket::Type type) noexcept -> int
{
    using Type = network::zeromq::socket::Type;

    static const auto map = robin_hood::unordered_flat_map<Type, int>{
        {Type::Request, ZMQ_REQ},
        {Type::Reply, ZMQ_REP},
        {Type::Publish, ZMQ_PUB},
        {Type::Subscribe, ZMQ_SUB},
        {Type::Pull, ZMQ_PULL},
        {Type::Push, ZMQ_PUSH},
        {Type::Pair, ZMQ_PAIR},
        {Type::Dealer, ZMQ_DEALER},
        {Type::Router, ZMQ_ROUTER},
    };

    try {

        return map.at(type);
    } catch (...) {

        return 0;
    }
}
}  // namespace opentxs

namespace opentxs::factory
{
auto ZMQSocket(
    const network::zeromq::Context& context,
    const network::zeromq::socket::Type type) noexcept
    -> network::zeromq::socket::Raw
{
    using Imp = network::zeromq::socket::implementation::Raw;

    return std::make_unique<Imp>(context, type).release();
}

auto ZMQSocketNull() noexcept -> network::zeromq::socket::Raw
{
    using ReturnType = network::zeromq::socket::Raw;

    return std::make_unique<ReturnType::Imp>().release();
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Raw::Raw(const Context& context, const socket::Type type) noexcept
    : Imp()
    , type_(type)
    , socket_(::zmq_socket(context, to_native(type_)), ::zmq_close)
    , bound_endpoints_()
    , connected_endpoints_()
{
    SetLinger(0);
    SetIncomingHWM(default_hwm_);
    SetOutgoingHWM(default_hwm_);
    SetSendTimeout(default_send_timeout_);
}

auto Raw::Bind(const char* endpoint) noexcept -> bool
{
    if (0 != ::zmq_bind(Native(), endpoint)) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;

        return false;
    } else {
        record_endpoint(bound_endpoints_);

        return true;
    }
}

auto Raw::ClearSubscriptions() noexcept -> bool
{
    if (0 != ::zmq_setsockopt(Native(), ZMQ_SUBSCRIBE, "", 0)) {
        std::cerr << "Failed to set ZMQ_SUBSCRIBE\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    } else {

        return true;
    }
}

auto Raw::Close() noexcept -> void
{
    Stop();
    socket_.reset();
}

auto Raw::Connect(const char* endpoint) noexcept -> bool
{
    if (0 != ::zmq_connect(Native(), endpoint)) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;

        return false;
    } else {
        record_endpoint(connected_endpoints_);

        return true;
    }
}

auto Raw::Disconnect(const char* endpoint) noexcept -> bool
{
    const auto rc = ::zmq_disconnect(Native(), endpoint);

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;
    }

    connected_endpoints_.erase(endpoint);

    return 0 == rc;
}

auto Raw::DisconnectAll() noexcept -> bool
{
    for (const auto& endpoint : connected_endpoints_) {
        ::zmq_disconnect(Native(), endpoint.c_str());
    }

    connected_endpoints_.clear();

    return true;
}

auto Raw::record_endpoint(Endpoints& out) noexcept -> void
{
    auto buffer = std::array<char, 256>{};
    auto bytes = buffer.size();
    const auto rc =
        ::zmq_getsockopt(Native(), ZMQ_LAST_ENDPOINT, buffer.data(), &bytes);

    assert(0 == rc);

    out.emplace(buffer.data(), bytes);
}

auto Raw::Send(Message&& msg) noexcept -> bool
{
    const auto sent = send(std::move(msg), ZMQ_DONTWAIT);

    OT_ASSERT(sent);

    return sent;
}

auto Raw::SendDeferred(Message&& msg) noexcept -> bool
{
    return send(std::move(msg), 0);
}

auto Raw::SendExternal(Message&& msg) noexcept -> bool
{
    return send(std::move(msg), ZMQ_DONTWAIT);
}

auto Raw::send(Message&& msg, const int baseFlags) noexcept -> bool
{
    auto sent{true};
    const auto parts = msg.size();
    auto counter = std::size_t{0};

    for (auto& frame : msg) {
        auto flags{baseFlags};

        if (++counter < parts) { flags |= ZMQ_SNDMORE; }

        sent &=
            (-1 != ::zmq_msg_send(const_cast<Frame&>(frame), Native(), flags));
    }

    if (false == sent) {
        std::cerr << (OT_PRETTY_CLASS())
                  << "Send error: " << ::zmq_strerror(zmq_errno()) << '\n';
        std::cerr << "bound_endpoints_: ";
        for (const auto& s : bound_endpoints_) { std::cerr << s << " "; }
        std::cerr << std::endl;
        std::cerr << "connected_endpoints_: ";
        for (const auto& s : connected_endpoints_) { std::cerr << s << " "; }
        std::cerr << std::endl;
    }

    return sent;
}

auto Raw::SetExposedUntrusted() noexcept -> bool
{
    auto output = SetIncomingHWM(untrusted_hwm_);
    output &= SetOutgoingHWM(untrusted_hwm_);
    output &= SetMaxMessageSize(untrusted_max_message_size_);

    return output;
}

auto Raw::SetIncomingHWM(int value) noexcept -> bool
{
    const auto rc =
        ::zmq_setsockopt(Native(), ZMQ_RCVHWM, &value, sizeof(value));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_RCVHWM\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetLinger(int value) noexcept -> bool
{
    const auto rc =
        ::zmq_setsockopt(Native(), ZMQ_LINGER, &value, sizeof(value));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_LINGER\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetMonitor(const char* endpoint, int events) noexcept -> bool
{
    const auto rc = ::zmq_socket_monitor(Native(), endpoint, events);

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed zmq_socket_monitor\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetMaxMessageSize(std::size_t arg) noexcept -> bool
{
    using ZMQArg = std::int64_t;

    if (std::numeric_limits<ZMQArg>::max() < arg) {
        std::cerr << (OT_PRETTY_CLASS()) << "Argument too large\n";

        return false;
    }

    const auto rc =
        ::zmq_setsockopt(Native(), ZMQ_MAXMSGSIZE, &arg, sizeof(arg));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_MAXMSGSIZE\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetOutgoingHWM(int value) noexcept -> bool
{
    const auto rc =
        ::zmq_setsockopt(Native(), ZMQ_SNDHWM, &value, sizeof(value));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_SNDHWM\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetPrivateKey(ReadView key) noexcept -> bool
{
    if (CURVE_KEY_BYTES != key.size()) {
        std::cerr << (OT_PRETTY_CLASS()) << "Invalid private key" << std::endl;

        return false;
    }

    const int server{1};
    auto rc =
        ::zmq_setsockopt(Native(), ZMQ_CURVE_SERVER, &server, sizeof(server));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_CURVE_SERVER"
                  << std::endl;

        return false;
    }

    rc =
        ::zmq_setsockopt(Native(), ZMQ_CURVE_SECRETKEY, key.data(), key.size());

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set private key"
                  << std::endl;

        return false;
    }

    return true;
}

auto Raw::SetRouterHandover(bool value) noexcept -> bool
{
    const auto data = value ? int{1} : int{0};
    const auto rc =
        ::zmq_setsockopt(Native(), ZMQ_ROUTER_HANDOVER, &data, sizeof(data));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS())
                  << "Failed to set ZMQ_ROUTER_HANDOVER\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetRoutingID(ReadView id) noexcept -> bool
{
    const auto set =
        ::zmq_setsockopt(Native(), ZMQ_ROUTING_ID, id.data(), id.size());

    if (0 != set) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;

        return false;
    }

    return true;
}

auto Raw::SetSendTimeout(std::chrono::milliseconds value) noexcept -> bool
{
    const auto ms = value.count();
    using ZMQArg = int;

    if (std::numeric_limits<ZMQArg>::max() < ms) {
        std::cerr << (OT_PRETTY_CLASS()) << "Argument too large\n";

        return false;
    }

    const auto arg = [&]() -> ZMQArg {
        if (0 > ms) {

            return -1;
        } else {
            return static_cast<ZMQArg>(ms);
        }
    }();
    const auto set =
        ::zmq_setsockopt(Native(), ZMQ_SNDTIMEO, &arg, sizeof(arg));

    if (0 != set) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;

        return false;
    }

    return true;
}

auto Raw::SetZAPDomain(ReadView domain) noexcept -> bool
{
    const auto set = ::zmq_setsockopt(
        Native(), ZMQ_ZAP_DOMAIN, domain.data(), domain.size());

    if (0 != set) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;

        return false;
    }

    return true;
}

auto Raw::Stop() noexcept -> void
{
    DisconnectAll();
    UnbindAll();
}

auto Raw::Unbind(const char* endpoint) noexcept -> bool
{
    const auto rc = ::zmq_unbind(Native(), endpoint);

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;
    }

    connected_endpoints_.erase(endpoint);

    return 0 == rc;
}

auto Raw::UnbindAll() noexcept -> bool
{
    auto output{true};

    for (const auto& endpoint : bound_endpoints_) {
        const auto rc = ::zmq_unbind(Native(), endpoint.c_str());

        if (0 != rc) {
            std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                      << std::endl;
            output = false;
        }
    }

    bound_endpoints_.clear();

    return output;
}

Raw::~Raw() { Close(); }
}  // namespace opentxs::network::zeromq::socket::implementation

namespace opentxs::network::zeromq::socket
{
auto swap(Raw& lhs, Raw& rhs) noexcept -> void { lhs.swap(rhs); }

Raw::Raw(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(imp);
}

Raw::Raw(Raw&& rhs) noexcept
    : Raw(std::make_unique<Imp>().release())
{
    swap(rhs);
}

auto Raw::operator=(Raw&& rhs) noexcept -> Raw&
{
    swap(rhs);

    return *this;
}

auto Raw::Bind(const char* endpoint) noexcept -> bool
{
    return imp_->Bind(endpoint);
}

auto Raw::ClearSubscriptions() noexcept -> bool
{
    return imp_->ClearSubscriptions();
}

auto Raw::Close() noexcept -> void { imp_->Close(); }

auto Raw::Connect(const char* endpoint) noexcept -> bool
{
    return imp_->Connect(endpoint);
}

auto Raw::Disconnect(const char* endpoint) noexcept -> bool
{
    return imp_->Disconnect(endpoint);
}

auto Raw::DisconnectAll() noexcept -> bool { return imp_->DisconnectAll(); }

auto Raw::ID() const noexcept -> SocketID { return imp_->ID(); }

auto Raw::Native() noexcept -> void* { return imp_->Native(); }

auto Raw::Send(Message&& msg) noexcept -> bool
{
    return imp_->Send(std::move(msg));
}

auto Raw::SendDeferred(Message&& msg) noexcept -> bool
{
    return imp_->SendDeferred(std::move(msg));
}

auto Raw::SendExternal(Message&& msg) noexcept -> bool
{
    return imp_->SendExternal(std::move(msg));
}

auto Raw::SetExposedUntrusted() noexcept -> bool
{
    return imp_->SetExposedUntrusted();
}

auto Raw::SetIncomingHWM(int value) noexcept -> bool
{
    return imp_->SetIncomingHWM(value);
}

auto Raw::SetLinger(int value) noexcept -> bool
{
    return imp_->SetLinger(value);
}

auto Raw::SetMonitor(const char* endpoint, int events) noexcept -> bool
{
    return imp_->SetMonitor(endpoint, events);
}

auto Raw::SetOutgoingHWM(int value) noexcept -> bool
{
    return imp_->SetOutgoingHWM(value);
}

auto Raw::SetPrivateKey(ReadView key) noexcept -> bool
{
    return imp_->SetPrivateKey(key);
}

auto Raw::SetRouterHandover(bool value) noexcept -> bool
{
    return imp_->SetRouterHandover(value);
}

auto Raw::SetRoutingID(ReadView id) noexcept -> bool
{
    return imp_->SetRoutingID(id);
}

auto Raw::SetSendTimeout(std::chrono::milliseconds value) noexcept -> bool
{
    return imp_->SetSendTimeout(value);
}

auto Raw::SetZAPDomain(ReadView domain) noexcept -> bool
{
    return imp_->SetZAPDomain(domain);
}

auto Raw::Stop() noexcept -> void { return imp_->Stop(); }

auto Raw::swap(Raw& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

auto Raw::Type() const noexcept -> socket::Type { return imp_->Type(); }

auto Raw::Unbind(const char* endpoint) noexcept -> bool
{
    return imp_->Unbind(endpoint);
}

auto Raw::UnbindAll() noexcept -> bool { return imp_->UnbindAll(); }

Raw::~Raw()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq::socket

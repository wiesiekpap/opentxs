// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_CONTEXT_HPP
#define OPENTXS_NETWORK_ZEROMQ_CONTEXT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace google
{
namespace protobuf
{
class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
class ListenCallback;
class PairEventCallback;
class Pipeline;
class Proxy;
class ReplyCallback;
}  // namespace zeromq
}  // namespace network

using OTZMQContext = Pimpl<network::zeromq::Context>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
class OPENTXS_EXPORT Context
{
public:
    static auto RawToZ85(
        const ReadView input,
        const AllocateOutput output) noexcept -> bool;
    static auto Z85ToRaw(
        const ReadView input,
        const AllocateOutput output) noexcept -> bool;

    virtual operator void*() const noexcept = 0;

    virtual auto BuildEndpoint(
        const std::string& path,
        const int instance,
        const int version) const noexcept -> std::string = 0;
    virtual auto BuildEndpoint(
        const std::string& path,
        const int instance,
        const int version,
        const std::string& suffix) const noexcept -> std::string = 0;
    virtual auto DealerSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept
        -> Pimpl<network::zeromq::socket::Dealer> = 0;
    template <
        typename Input,
        std::enable_if_t<
            std::is_pointer<decltype(std::declval<Input&>().data())>::value,
            int> = 0,
        std::enable_if_t<
            std::is_integral<decltype(std::declval<Input&>().size())>::value,
            int> = 0>
    auto Frame(const Input& input) const noexcept -> OTZMQFrame
    {
        return Frame(input.data(), input.size());
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    auto Frame(const Input& input) const noexcept -> OTZMQFrame
    {
        return Frame(&input, sizeof(input));
    }
    virtual auto Frame(const void* input, const std::size_t size) const noexcept
        -> Pimpl<network::zeromq::Frame> = 0;
    virtual auto Message() const noexcept
        -> Pimpl<network::zeromq::Message> = 0;
    OPENTXS_NO_EXPORT virtual auto Message(
        const ::google::protobuf::MessageLite& input) const noexcept
        -> Pimpl<network::zeromq::Message> = 0;
    virtual auto Message(const network::zeromq::Message& input) const noexcept
        -> Pimpl<network::zeromq::Message> = 0;
    template <
        typename Input,
        std::enable_if_t<
            std::is_pointer<decltype(std::declval<Input&>().data())>::value,
            int> = 0,
        std::enable_if_t<
            std::is_integral<decltype(std::declval<Input&>().size())>::value,
            int> = 0>
    auto Message(const Input& input) const noexcept
        -> Pimpl<network::zeromq::Message>
    {
        return Message(input.data(), input.size());
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    auto Message(const Input& input) const noexcept
        -> Pimpl<network::zeromq::Message>
    {
        return Message(&input, sizeof(input));
    }
    template <typename Input>
    auto Message(const Pimpl<Input>& input) const noexcept
        -> Pimpl<network::zeromq::Message>
    {
        return Message(input.get());
    }
    virtual auto Message(const void* input, const std::size_t size)
        const noexcept -> Pimpl<network::zeromq::Message> = 0;
    virtual auto PairEventListener(
        const PairEventCallback& callback,
        const int instance) const noexcept
        -> Pimpl<network::zeromq::socket::Subscribe> = 0;
    virtual auto PairSocket(const ListenCallback& callback) const noexcept
        -> Pimpl<network::zeromq::socket::Pair> = 0;
    virtual auto PairSocket(
        const ListenCallback& callback,
        const zeromq::socket::Pair& peer) const noexcept
        -> Pimpl<network::zeromq::socket::Pair> = 0;
    virtual auto PairSocket(
        const ListenCallback& callback,
        const std::string& endpoint) const noexcept
        -> Pimpl<network::zeromq::socket::Pair> = 0;
    virtual auto Pipeline(
        const api::Core& api,
        std::function<void(zeromq::Message&)> callback) const noexcept
        -> Pimpl<network::zeromq::Pipeline> = 0;
    virtual auto Proxy(socket::Socket& frontend, socket::Socket& backend)
        const noexcept -> Pimpl<network::zeromq::Proxy> = 0;
    virtual auto PublishSocket() const noexcept
        -> Pimpl<network::zeromq::socket::Publish> = 0;
    virtual auto PullSocket(const socket::Socket::Direction direction)
        const noexcept -> Pimpl<network::zeromq::socket::Pull> = 0;
    virtual auto PullSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept
        -> Pimpl<network::zeromq::socket::Pull> = 0;
    virtual auto PushSocket(const socket::Socket::Direction direction)
        const noexcept -> Pimpl<network::zeromq::socket::Push> = 0;
    virtual auto ReplyMessage(const zeromq::Message& request) const noexcept
        -> Pimpl<network::zeromq::Message> = 0;
    virtual auto ReplyMessage(const ReadView connectionID) const noexcept
        -> Pimpl<network::zeromq::Message> = 0;
    virtual auto ReplySocket(
        const ReplyCallback& callback,
        const socket::Socket::Direction direction) const noexcept
        -> Pimpl<network::zeromq::socket::Reply> = 0;
    virtual auto RequestSocket() const noexcept
        -> Pimpl<network::zeromq::socket::Request> = 0;
    virtual auto RouterSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept
        -> Pimpl<network::zeromq::socket::Router> = 0;
    virtual auto SubscribeSocket(const ListenCallback& callback) const noexcept
        -> Pimpl<network::zeromq::socket::Subscribe> = 0;
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    auto TaggedMessage(const Input& tag) const noexcept
        -> Pimpl<network::zeromq::Message>
    {
        return TaggedMessage(&tag, sizeof(tag));
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    auto TaggedReply(const zeromq::Message& request, const Input& tag)
        const noexcept -> Pimpl<network::zeromq::Message>
    {
        return TaggedReply(request, &tag, sizeof(tag));
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    auto TaggedReply(const ReadView connectionID, const Input& tag)
        const noexcept -> Pimpl<network::zeromq::Message>
    {
        return TaggedReply(connectionID, &tag, sizeof(tag));
    }

    virtual ~Context() = default;

protected:
    Context() noexcept = default;

private:
    friend OTZMQContext;

    virtual auto clone() const noexcept -> Context* = 0;
    virtual auto TaggedMessage(const void* tag, const std::size_t size)
        const noexcept -> Pimpl<network::zeromq::Message> = 0;
    virtual auto TaggedReply(
        const zeromq::Message& request,
        const void* tag,
        const std::size_t size) const noexcept
        -> Pimpl<network::zeromq::Message> = 0;
    virtual auto TaggedReply(
        const ReadView connectionID,
        const void* tag,
        const std::size_t size) const noexcept
        -> Pimpl<network::zeromq::Message> = 0;

    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

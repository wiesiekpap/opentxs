// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_CONTEXT_HPP
#define OPENTXS_NETWORK_ZEROMQ_CONTEXT_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
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

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::Pimpl(opentxs::network::zeromq::Context const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator opentxs::network::zeromq::Context&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator const opentxs::network::zeromq::Context &;
%ignore opentxs::network::zeromq::Context::operator void*() const;
%ignore opentxs::network::zeromq::Context::Pipeline const;
%rename(assign) operator=(const opentxs::network::zeromq::Context&);
%rename(ZMQContext) opentxs::network::zeromq::Context;
%template(OTZMQContext) opentxs::Pimpl<opentxs::network::zeromq::Context>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
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
class Context
{
public:
    OPENTXS_EXPORT static bool RawToZ85(
        const ReadView input,
        const AllocateOutput output) noexcept;
    OPENTXS_EXPORT static bool Z85ToRaw(
        const ReadView input,
        const AllocateOutput output) noexcept;

    OPENTXS_EXPORT virtual operator void*() const noexcept = 0;

    OPENTXS_EXPORT virtual std::string BuildEndpoint(
        const std::string& path,
        const int instance,
        const int version) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string BuildEndpoint(
        const std::string& path,
        const int instance,
        const int version,
        const std::string& suffix) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Dealer> DealerSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept = 0;
#ifndef SWIG
    template <
        typename Input,
        std::enable_if_t<
            std::is_pointer<decltype(std::declval<Input&>().data())>::value,
            int> = 0,
        std::enable_if_t<
            std::is_integral<decltype(std::declval<Input&>().size())>::value,
            int> = 0>
    OPENTXS_EXPORT OTZMQFrame Frame(const Input& input) const noexcept
    {
        return Frame(input.data(), input.size());
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    OPENTXS_EXPORT OTZMQFrame Frame(const Input& input) const noexcept
    {
        return Frame(&input, sizeof(input));
    }
#endif
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Frame> Frame(
        const void* input,
        const std::size_t size) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Message> Message()
        const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Message> Message(
        const ProtobufType& input) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Message> Message(
        const network::zeromq::Message& input) const noexcept = 0;
#ifndef SWIG
    template <
        typename Input,
        std::enable_if_t<
            std::is_pointer<decltype(std::declval<Input&>().data())>::value,
            int> = 0,
        std::enable_if_t<
            std::is_integral<decltype(std::declval<Input&>().size())>::value,
            int> = 0>
    OPENTXS_EXPORT Pimpl<network::zeromq::Message> Message(
        const Input& input) const noexcept
    {
        return Message(input.data(), input.size());
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    OPENTXS_EXPORT Pimpl<network::zeromq::Message> Message(
        const Input& input) const noexcept
    {
        return Message(&input, sizeof(input));
    }
    template <typename Input>
    OPENTXS_EXPORT Pimpl<network::zeromq::Message> Message(
        const Pimpl<Input>& input) const noexcept
    {
        return Message(input.get());
    }
#endif
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Message> Message(
        const void* input,
        const std::size_t size) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Subscribe>
    PairEventListener(const PairEventCallback& callback, const int instance)
        const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Pair> PairSocket(
        const ListenCallback& callback) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Pair> PairSocket(
        const ListenCallback& callback,
        const zeromq::socket::Pair& peer) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Pair> PairSocket(
        const ListenCallback& callback,
        const std::string& endpoint) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Pipeline> Pipeline(
        const api::internal::Core& api,
        std::function<void(zeromq::Message&)> callback) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Proxy> Proxy(
        socket::Socket& frontend,
        socket::Socket& backend) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Publish>
    PublishSocket() const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Pull> PullSocket(
        const socket::Socket::Direction direction) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Pull> PullSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Push> PushSocket(
        const socket::Socket::Direction direction) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Message> ReplyMessage(
        const zeromq::Message& request) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Message> ReplyMessage(
        const ReadView connectionID) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Reply> ReplySocket(
        const ReplyCallback& callback,
        const socket::Socket::Direction direction) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Request>
    RequestSocket() const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Router> RouterSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::socket::Subscribe>
    SubscribeSocket(const ListenCallback& callback) const noexcept = 0;
#ifndef SWIG
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    OPENTXS_EXPORT Pimpl<network::zeromq::Message> TaggedMessage(
        const Input& tag) const noexcept
    {
        return TaggedMessage(&tag, sizeof(tag));
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    OPENTXS_EXPORT Pimpl<network::zeromq::Message> TaggedReply(
        const zeromq::Message& request,
        const Input& tag) const noexcept
    {
        return TaggedReply(request, &tag, sizeof(tag));
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    OPENTXS_EXPORT Pimpl<network::zeromq::Message> TaggedReply(
        const ReadView connectionID,
        const Input& tag) const noexcept
    {
        return TaggedReply(connectionID, &tag, sizeof(tag));
    }
#endif

    OPENTXS_EXPORT virtual ~Context() = default;

protected:
    Context() noexcept = default;

private:
    friend OTZMQContext;

    OPENTXS_EXPORT virtual Context* clone() const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Message> TaggedMessage(
        const void* tag,
        const std::size_t size) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Message> TaggedReply(
        const zeromq::Message& request,
        const void* tag,
        const std::size_t size) const noexcept = 0;
    OPENTXS_EXPORT virtual Pimpl<network::zeromq::Message> TaggedReply(
        const ReadView connectionID,
        const void* tag,
        const std::size_t size) const noexcept = 0;

    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

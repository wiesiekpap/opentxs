// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_MESSAGE_HPP
#define OPENTXS_NETWORK_ZEROMQ_MESSAGE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/Frame.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::Message>::Pimpl(opentxs::network::zeromq::Message const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::Message>::operator opentxs::network::zeromq::Message&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Message>::operator const opentxs::network::zeromq::Message &;
%ignore opentxs::network::zeromq::Message::at(const std::size_t) const;
%ignore opentxs::network::zeromq::Message::begin() const;
%ignore opentxs::network::zeromq::Message::end() const;
%rename(assign) operator=(const opentxs::network::zeromq::Message&);
%rename(ZMQMessage) opentxs::network::zeromq::Message;
%template(OTZMQMessage) opentxs::Pimpl<opentxs::network::zeromq::Message>;
// clang-format on
#endif  // SWIG

namespace google
{
namespace protobuf
{
class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace opentxs
{
namespace network
{
namespace zeromq
{
class FrameIterator;
class FrameSection;
class Message;
}  // namespace zeromq
}  // namespace network

using OTZMQMessage = Pimpl<network::zeromq::Message>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
class OPENTXS_EXPORT Message
{
public:
    static Pimpl<Message> Factory();

    virtual const Frame& at(const std::size_t index) const = 0;
    virtual FrameIterator begin() const = 0;
    virtual const FrameSection Body() const = 0;
    virtual const Frame& Body_at(const std::size_t index) const = 0;
    virtual FrameIterator Body_begin() const = 0;
    virtual FrameIterator Body_end() const = 0;
    virtual FrameIterator end() const = 0;
    virtual const FrameSection Header() const = 0;
    virtual const Frame& Header_at(const std::size_t index) const = 0;
    virtual FrameIterator Header_begin() const = 0;
    virtual FrameIterator Header_end() const = 0;
    virtual std::size_t size() const = 0;

    virtual Frame& AddFrame() = 0;
#ifndef SWIG
    OPENTXS_NO_EXPORT virtual Frame& AddFrame(
        const ::google::protobuf::MessageLite& input) = 0;
    template <
        typename Input,
        std::enable_if_t<
            std::is_pointer<decltype(std::declval<Input&>().data())>::value,
            int> = 0,
        std::enable_if_t<
            std::is_integral<decltype(std::declval<Input&>().size())>::value,
            int> = 0>
    auto AddFrame(const Input& input) -> Frame&
    {
        return AddFrame(input.data(), input.size());
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    auto AddFrame(const Input& input) -> Frame&
    {
        return AddFrame(&input, sizeof(input));
    }
    template <typename Input>
    auto AddFrame(const Pimpl<Input>& input) -> Frame&
    {
        return AddFrame(input.get());
    }
    virtual auto AddFrame(const void* input, const std::size_t size)
        -> Frame& = 0;
#endif
    virtual AllocateOutput AppendBytes() noexcept = 0;
    virtual Frame& at(const std::size_t index) = 0;
    virtual FrameSection Body() = 0;
    virtual void EnsureDelimiter() = 0;
    virtual FrameSection Header() = 0;
    virtual void PrependEmptyFrame() = 0;
    virtual Frame& Replace(const std::size_t index, OTZMQFrame&& frame) = 0;
    virtual void StartBody() noexcept = 0;

    virtual ~Message() = default;

protected:
    Message() = default;

private:
    friend OTZMQMessage;

#ifdef _WIN32
public:
#endif
    virtual Message* clone() const = 0;
#ifdef _WIN32
private:
#endif

    Message(const Message&) = delete;
    Message(Message&&) = default;
    Message& operator=(const Message&) = delete;
    Message& operator=(Message&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

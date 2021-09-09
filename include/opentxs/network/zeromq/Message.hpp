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
    static auto Factory() -> Pimpl<Message>;

    virtual auto at(const std::size_t index) const -> const Frame& = 0;
    virtual auto begin() const -> FrameIterator = 0;
    virtual auto Body() const -> const FrameSection = 0;
    virtual auto Body_at(const std::size_t index) const -> const Frame& = 0;
    virtual auto Body_begin() const -> FrameIterator = 0;
    virtual auto Body_end() const -> FrameIterator = 0;
    virtual auto end() const -> FrameIterator = 0;
    virtual auto Header() const -> const FrameSection = 0;
    virtual auto Header_at(const std::size_t index) const -> const Frame& = 0;
    virtual auto Header_begin() const -> FrameIterator = 0;
    virtual auto Header_end() const -> FrameIterator = 0;
    virtual auto size() const -> std::size_t = 0;
    virtual auto Total() const -> std::size_t = 0;

    virtual auto AddFrame() -> Frame& = 0;
    OPENTXS_NO_EXPORT virtual auto AddFrame(
        const ::google::protobuf::MessageLite& input) -> Frame& = 0;
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
    virtual auto AppendBytes() noexcept -> AllocateOutput = 0;
    virtual auto at(const std::size_t index) -> Frame& = 0;
    virtual auto Body() -> FrameSection = 0;
    virtual void EnsureDelimiter() = 0;
    virtual auto Header() -> FrameSection = 0;
    virtual void PrependEmptyFrame() = 0;
    virtual auto Replace(const std::size_t index, OTZMQFrame&& frame)
        -> Frame& = 0;
    virtual void StartBody() noexcept = 0;

    virtual ~Message() = default;

protected:
    Message() = default;

private:
    friend OTZMQMessage;

#ifdef _WIN32
public:
#endif
    virtual auto clone() const -> Message* = 0;
#ifdef _WIN32
private:
#endif

    Message(const Message&) = delete;
    Message(Message&&) = default;
    auto operator=(const Message&) -> Message& = delete;
    auto operator=(Message&&) -> Message& = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

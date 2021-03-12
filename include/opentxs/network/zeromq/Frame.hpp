// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_FRAME_HPP
#define OPENTXS_NETWORK_ZEROMQ_FRAME_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstring>  // IWYU pragma: keep
#include <string>
#include <type_traits>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"

struct zmq_msg_t;

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::Frame::bytes;
%ignore opentxs::network::zeromq::Frame::data;
%ignore opentxs::network::zeromq::Frame::operator zmq_msg_t*;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Frame>::Pimpl(opentxs::network::zeromq::Frame const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::Frame>::operator opentxs::network::zeromq::Frame&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Frame>::operator const opentxs::network::zeromq::Frame &;
%rename(string) opentxs::network::zeromq::Frame::operator std::string() const;
%rename(assign) operator=(const opentxs::network::zeromq::Frame&);
%rename(ZMQFrame) opentxs::network::zeromq::Frame;
%template(OTZMQFrame) opentxs::Pimpl<opentxs::network::zeromq::Frame>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network

using OTZMQFrame = Pimpl<network::zeromq::Frame>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Frame
{
public:
    OPENTXS_EXPORT virtual operator std::string() const noexcept = 0;

#ifndef SWIG
    template <
        typename Output,
        std::enable_if_t<std::is_trivially_copyable<Output>::value, int> = 0>
    auto as() const noexcept(false) -> Output
    {
        if (sizeof(Output) != size()) {
            auto error = std::string{"Invalid frame size: "} +
                         std::to_string(size()) +
                         " expected: " + std::to_string(sizeof(Output));

            throw std::runtime_error(error);
        }

        Output output{};
        std::memcpy(&output, data(), sizeof(output));

        return output;
    }
#endif

    OPENTXS_EXPORT virtual auto Bytes() const noexcept -> ReadView = 0;
#ifndef SWIG
    OPENTXS_EXPORT virtual auto data() const noexcept -> const void* = 0;
#endif
    OPENTXS_EXPORT virtual auto size() const noexcept -> std::size_t = 0;

    OPENTXS_EXPORT virtual operator zmq_msg_t*() noexcept = 0;

    OPENTXS_EXPORT virtual ~Frame() = default;

protected:
    Frame() = default;

private:
    friend OTZMQFrame;

    OPENTXS_EXPORT virtual Frame* clone() const noexcept = 0;

    Frame(const Frame&) = delete;
    Frame(Frame&&) = delete;
    Frame& operator=(Frame&&) = delete;
    Frame& operator=(const Frame&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

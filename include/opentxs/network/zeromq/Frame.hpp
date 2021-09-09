// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_FRAME_HPP
#define OPENTXS_NETWORK_ZEROMQ_FRAME_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstring>  // IWYU pragma: keep
#include <string>
#include <type_traits>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"

struct zmq_msg_t;

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
class OPENTXS_EXPORT Frame
{
public:
    virtual operator std::string() const noexcept = 0;

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

    virtual auto Bytes() const noexcept -> ReadView = 0;
    virtual auto data() const noexcept -> const void* = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    virtual operator zmq_msg_t*() noexcept = 0;

    virtual ~Frame() = default;

protected:
    Frame() = default;

private:
    friend OTZMQFrame;

    virtual auto clone() const noexcept -> Frame* = 0;

    Frame(const Frame&) = delete;
    Frame(Frame&&) = delete;
    auto operator=(Frame&&) -> Frame& = delete;
    auto operator=(const Frame&) -> Frame& = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

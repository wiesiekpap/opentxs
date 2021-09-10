// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_FRAMEITERATOR_HPP
#define OPENTXS_NETWORK_ZEROMQ_FRAMEITERATOR_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <atomic>
#include <cstddef>
#include <iosfwd>
#include <iterator>

#include "opentxs/network/zeromq/Frame.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;

class OPENTXS_EXPORT FrameIterator
{
public:
    using difference_type = std::size_t;
    using value_type = Frame;
    using pointer = Frame*;
    using reference = Frame&;
    using iterator_category = std::forward_iterator_tag;

    FrameIterator();
    FrameIterator(const FrameIterator&);
    FrameIterator(FrameIterator&&);
    FrameIterator(const Message* parent, std::size_t position = 0);
    auto operator=(const FrameIterator&) -> FrameIterator&;

    auto operator*() const -> const opentxs::network::zeromq::Frame&;
    auto operator==(const FrameIterator&) const -> bool;
    auto operator!=(const FrameIterator&) const -> bool;

    auto operator*() -> opentxs::network::zeromq::Frame&;
    auto operator++() -> FrameIterator&;
    auto operator++(int) -> FrameIterator;

    ~FrameIterator() = default;

private:
    std::atomic<std::size_t> position_{0};
    const Message* parent_{nullptr};

    auto operator=(FrameIterator&&) -> FrameIterator& = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

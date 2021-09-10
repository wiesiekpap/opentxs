// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_FRAMESECTION_HPP
#define OPENTXS_NETWORK_ZEROMQ_FRAMESECTION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iosfwd>
#include <iterator>

#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;

class OPENTXS_EXPORT FrameSection
{
public:
    using difference_type = std::size_t;
    using value_type = Frame;
    using pointer = Frame*;
    using reference = Frame&;
    using iterator_category = std::forward_iterator_tag;

    auto at(const std::size_t index) const -> const Frame&;
    auto begin() const -> FrameIterator;
    auto end() const -> FrameIterator;
    auto size() const -> std::size_t;

    auto Replace(const std::size_t index, OTZMQFrame&& frame) -> Frame&;

    FrameSection(const Message* parent, std::size_t position, std::size_t size);
    FrameSection(const FrameSection&);

    ~FrameSection() = default;

private:
    const Message* parent_;
    const std::size_t position_;
    const std::size_t size_;

    FrameSection() = delete;
    FrameSection(FrameSection&&) = delete;
    auto operator=(const FrameSection&) -> FrameSection& = delete;
    auto operator=(FrameSection&&) -> FrameSection& = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_FRAMESECTION_HPP
#define OPENTXS_NETWORK_ZEROMQ_FRAMESECTION_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <iosfwd>
#include <iterator>

#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"

#ifdef SWIG
// clang-format off
%rename(ZMQFrameSection) opentxs::network::zeromq::FrameSection;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;

class FrameSection
{
public:
    using difference_type = std::size_t;
    using value_type = Frame;
    using pointer = Frame*;
    using reference = Frame&;
    using iterator_category = std::forward_iterator_tag;

    OPENTXS_EXPORT const Frame& at(const std::size_t index) const;
    OPENTXS_EXPORT FrameIterator begin() const;
    OPENTXS_EXPORT FrameIterator end() const;
    OPENTXS_EXPORT std::size_t size() const;

    OPENTXS_EXPORT Frame& Replace(const std::size_t index, OTZMQFrame&& frame);

#ifndef SWIG
    OPENTXS_EXPORT FrameSection(
        const Message* parent,
        std::size_t position,
        std::size_t size);
#endif
    OPENTXS_EXPORT FrameSection(const FrameSection&);

    OPENTXS_EXPORT ~FrameSection() = default;

private:
    const Message* parent_;
    const std::size_t position_;
    const std::size_t size_;

    FrameSection() = delete;
    FrameSection(FrameSection&&) = delete;
    FrameSection& operator=(const FrameSection&) = delete;
    FrameSection& operator=(FrameSection&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

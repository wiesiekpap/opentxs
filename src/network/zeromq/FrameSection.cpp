// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "opentxs/network/zeromq/FrameSection.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs::network::zeromq
{
FrameSection::FrameSection(const FrameSection& frameSection)
    : parent_(frameSection.parent_)
    , position_(frameSection.position_)
    , size_(frameSection.size_)
{
    OT_ASSERT(nullptr != parent_);
}

FrameSection::FrameSection(
    const Message* parent,
    std::size_t position,
    std::size_t size)
    : parent_(parent)
    , position_(position)
    , size_(size)
{
    OT_ASSERT(nullptr != parent_);
}

auto FrameSection::at(const std::size_t index) const -> const Frame&
{
    OT_ASSERT(size_ > index);

    return parent_->at(position_ + index);
}

auto FrameSection::begin() const -> FrameIterator
{
    return FrameIterator(parent_, position_);
}

auto FrameSection::end() const -> FrameIterator
{
    return FrameIterator(parent_, position_ + size_);
}

auto FrameSection::Replace(const std::size_t index, OTZMQFrame&& frame)
    -> Frame&
{
    return const_cast<Message&>(*parent_).Replace(
        position_ + index, std::move(frame));
}

auto FrameSection::size() const -> std::size_t { return size_; }
}  // namespace opentxs::network::zeromq

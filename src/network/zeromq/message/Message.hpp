// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <optional>

#include "Proto.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/message/Factory.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::network::zeromq
{
class Message::Imp : virtual public internal::Message
{
public:
    zeromq::Message* parent_;

    auto at(const std::size_t index) const noexcept(false) -> const Frame&;
    auto begin() const noexcept -> const FrameIterator;
    auto Body() const noexcept -> const FrameSection;
    auto Body_at(const std::size_t index) const noexcept(false) -> const Frame&;
    auto Body_begin() const noexcept -> const FrameIterator;
    auto Body_end() const noexcept -> const FrameIterator;
    auto body_position() const noexcept -> std::size_t;
    auto end() const noexcept -> const FrameIterator;
    auto Header() const noexcept -> const FrameSection;
    auto Header_at(const std::size_t index) const noexcept(false)
        -> const Frame&;
    auto Header_begin() const noexcept -> const FrameIterator;
    auto Header_end() const noexcept -> const FrameIterator;
    auto operator<(const zeromq::Message& rhs) const noexcept -> bool;
    auto operator==(const zeromq::Message& rhs) const noexcept -> bool;
    auto size() const noexcept -> std::size_t;
    auto Total() const noexcept -> std::size_t;

    auto AddFrame() noexcept -> Frame&;
    auto AddFrame(const Amount& amount) noexcept -> Frame&;
    auto AddFrame(Frame&& frame) noexcept -> Frame&;
    auto AddFrame(const char* in) noexcept -> Frame&;
    auto AddFrame(const ProtobufType& input) noexcept -> Frame& final;
    auto AddFrame(const ReadView bytes) noexcept -> Frame&;
    auto AddFrame(const void* input, const std::size_t size) noexcept -> Frame&;
    auto PopFrame() -> void;
    auto AppendBytes() noexcept -> AllocateOutput;
    auto at(const std::size_t index) -> Frame&;
    auto Body() noexcept -> FrameSection;
    auto EnsureDelimiter() noexcept -> void;
    auto ExtractFront() noexcept -> zeromq::Frame final;
    auto Header() noexcept -> FrameSection;
    auto Prepend(SocketID id) noexcept -> zeromq::Frame& final;
    auto StartBody() noexcept -> void;

    Imp() noexcept;
    Imp(const Imp& rhs) noexcept;

    ~Imp() override = default;

protected:
    UnallocatedVector<Frame> frames_;

    auto set_field(
        const std::size_t position,
        const zeromq::Frame& input) noexcept -> bool;

private:
    auto hasDivider() const noexcept -> bool;
    auto findDivider() const noexcept -> std::size_t;

    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::network::zeromq

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/Message.hpp"

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace ot = opentxs;

namespace ottest
{
struct ZMQQueue {
    using Message = ot::network::zeromq::Message;

    auto get(std::size_t index) noexcept(false) -> const Message&;
    auto receive(const Message& msg) noexcept -> void;

    ZMQQueue() noexcept;

    ~ZMQQueue();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace ottest

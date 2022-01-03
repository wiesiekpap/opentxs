// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <mutex>

#include "internal/util/Timer.hpp"
#include "opentxs/util/Time.hpp"

namespace boost
{
namespace system
{
class error_code;
}  // namespace system
}  // namespace boost

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
class Pipeline;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::blockchain::p2p::peer
{
class Activity
{
public:
    auto get() const noexcept -> Time;

    auto Bump() noexcept -> void;

    Activity(
        const api::Session& api,
        const network::zeromq::Pipeline& pipeline) noexcept;

private:
    static constexpr auto ping_interval_ = std::chrono::seconds{15};
    static constexpr auto activity_timeout_ = ping_interval_ * 3;

    const network::zeromq::Pipeline& pipeline_;
    mutable std::mutex lock_;
    Timer disconnect_;
    Timer ping_;
    Time activity_;

    auto reset_disconnect() noexcept -> void;
    auto reset_ping() noexcept -> void;
};
}  // namespace opentxs::blockchain::p2p::peer

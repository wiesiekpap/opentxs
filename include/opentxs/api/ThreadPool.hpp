// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_THREAD_POOL_HPP
#define OPENTXS_API_THREAD_POOL_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <functional>
#include <string>

#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class OPENTXS_EXPORT ThreadPool
{
public:
    using Message = opentxs::network::zeromq::Message;
    using Callback = std::function<void(const Message&)>;
    using WorkType = OTZMQWorkType;

    static auto Capacity() noexcept -> std::size_t;
    static auto MakeWork(
        const opentxs::network::zeromq::Context& zmq,
        WorkType type) noexcept -> OTZMQMessage;

    virtual auto Endpoint() const noexcept -> std::string = 0;
    virtual auto Register(WorkType type, Callback handler) const noexcept
        -> bool = 0;

    virtual ~ThreadPool() = default;

protected:
    ThreadPool() = default;

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif

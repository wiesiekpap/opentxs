// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <thread>
#include <tuple>

#include "internal/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Handle;
class Thread;
}  // namespace internal

class Pipeline;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::internal
{
class Context : virtual public zeromq::Context
{
public:
    virtual auto BelongsToThreadPool(
        const std::thread::id = std::this_thread::get_id()) const noexcept
        -> bool = 0;
    auto Internal() const noexcept -> const internal::Context& final
    {
        return *this;
    }
    virtual auto MakeBatch(
        UnallocatedVector<socket::Type>&& types) const noexcept -> Handle = 0;
    virtual auto Modify(SocketID id, ModifyCallback cb) const noexcept
        -> std::pair<bool, std::future<bool>> = 0;
    virtual auto Pipeline(
        std::function<void(zeromq::Message&&)> callback,
        const EndpointArgs& subscribe = {},
        const EndpointArgs& pull = {},
        const EndpointArgs& dealer = {}) const noexcept -> zeromq::Pipeline = 0;
    virtual auto Start(BatchID id, StartArgs&& sockets) const noexcept
        -> Thread* = 0;
    virtual auto Thread(BatchID id) const noexcept -> Thread* = 0;
    virtual auto ThreadID(BatchID id) const noexcept -> std::thread::id = 0;

    auto Internal() noexcept -> internal::Context& final { return *this; }

    ~Context() override = default;

private:
    friend Handle;

    virtual auto Stop(BatchID id) const noexcept -> std::future<bool> = 0;
};
}  // namespace opentxs::network::zeromq::internal

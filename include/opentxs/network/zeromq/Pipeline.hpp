// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Pipeline;
}  // namespace internal

class Context;
class Message;
class Pipeline;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq
{
OPENTXS_EXPORT auto swap(Pipeline& lhs, Pipeline& rhs) noexcept -> void;

class OPENTXS_EXPORT Pipeline
{
public:
    class Imp;

    auto Close() const noexcept -> bool;
    auto ConnectDealer(
        const UnallocatedCString& endpoint,
        std::function<Message(bool)> notify = {}) const noexcept -> bool;
    auto ConnectionIDDealer() const noexcept -> std::size_t;
    auto ConnectionIDInternal() const noexcept -> std::size_t;
    auto ConnectionIDPull() const noexcept -> std::size_t;
    auto ConnectionIDSubscribe() const noexcept -> std::size_t;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Pipeline&;
    auto PullFrom(const UnallocatedCString& endpoint) const noexcept -> bool;
    auto Push(Message&& msg) const noexcept -> bool;
    auto Send(Message&& msg) const noexcept -> bool;
    auto SubscribeTo(const UnallocatedCString& endpoint) const noexcept -> bool;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Pipeline&;
    virtual auto swap(Pipeline& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Pipeline(Imp* imp) noexcept;
    Pipeline(Pipeline&& rhs) noexcept;
    auto operator=(Pipeline&& rhs) noexcept -> Pipeline&;

    virtual ~Pipeline();

private:
    Imp* imp_;

    Pipeline() = delete;
    Pipeline(const Pipeline&) = delete;
    auto operator=(const Pipeline&) -> Pipeline& = delete;
};
}  // namespace opentxs::network::zeromq

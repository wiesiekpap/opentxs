// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_PIPELINE_HPP
#define OPENTXS_NETWORK_ZEROMQ_PIPELINE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Pipeline;
}  // namespace zeromq
}  // namespace network

using OTZMQPipeline = Pimpl<network::zeromq::Pipeline>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
class OPENTXS_EXPORT Pipeline
{
public:
    virtual auto Close() const noexcept -> bool = 0;
    virtual auto Context() const noexcept -> const zeromq::Context& = 0;
    template <typename Input>
    auto Push(const Input& data) const noexcept -> bool
    {
        return push(Context().Message(data));
    }
    virtual void Start(const std::string& endpoint) const noexcept = 0;

    virtual ~Pipeline() = default;

protected:
    Pipeline() noexcept = default;

private:
    friend OTZMQPipeline;

    virtual auto clone() const noexcept -> Pipeline* = 0;
    virtual auto push(network::zeromq::Message& data) const noexcept
        -> bool = 0;

    Pipeline(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    auto operator=(const Pipeline&) -> Pipeline& = delete;
    auto operator=(Pipeline&&) -> Pipeline& = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

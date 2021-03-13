// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "core/Shutdown.hpp"
#include "opentxs/api/Endpoints.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::api::implementation
{
class ZMQ
{
public:
    virtual ~ZMQ() = default;

protected:
    const opentxs::network::zeromq::Context& zmq_context_;
    const int instance_;

private:
    std::unique_ptr<api::Endpoints> endpoints_p_;

protected:
    const api::Endpoints& endpoints_;
    opentxs::internal::ShutdownSender shutdown_sender_;

    ZMQ(const opentxs::network::zeromq::Context& zmq, const int instance)
    noexcept;

private:
    ZMQ() = delete;
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    auto operator=(const ZMQ&) -> ZMQ& = delete;
    auto operator=(ZMQ&&) -> ZMQ& = delete;
};
}  // namespace opentxs::api::implementation

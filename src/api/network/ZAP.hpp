// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "opentxs/api/network/ZAP.hpp"
#include "opentxs/network/zeromq/zap/Callback.hpp"
#include "opentxs/network/zeromq/zap/Handler.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

class Factory;
}  // namespace opentxs

namespace opentxs::api::network::implementation
{
class ZAP final : virtual public api::network::ZAP
{
public:
    auto RegisterDomain(const std::string& domain, const Callback& callback)
        const -> bool final;
    auto SetDefaultPolicy(const Policy policy) const -> bool final;

    ~ZAP() final = default;

private:
    friend opentxs::Factory;

    const opentxs::network::zeromq::Context& context_;
    OTZMQZAPCallback callback_;
    OTZMQZAPHandler zap_;

    ZAP(const opentxs::network::zeromq::Context& context);
    ZAP() = delete;
    ZAP(const ZAP&) = delete;
    ZAP(ZAP&&) = delete;
    auto operator=(const ZAP&) -> ZAP& = delete;
    auto operator=(ZAP&&) -> ZAP& = delete;
};
}  // namespace opentxs::api::network::implementation

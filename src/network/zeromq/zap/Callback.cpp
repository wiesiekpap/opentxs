// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "network/zeromq/zap/Callback.hpp"  // IWYU pragma: associated

#include <atomic>
#include <functional>
#include <memory>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/zap/Reply.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"
#include "opentxs/network/zeromq/zap/ZAP.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::zap::Callback>;

#define OT_METHOD "opentxs::network::zeromq::zap::implementation::Callback::"

namespace opentxs::network::zeromq::zap
{
auto Callback::Factory(
    const std::string& domain,
    const ReceiveCallback& callback) -> OTZMQZAPCallback
{
    auto output = OTZMQZAPCallback(new implementation::Callback());
    output->SetDomain(domain, callback);

    return output;
}

auto Callback::Factory() -> OTZMQZAPCallback
{
    return OTZMQZAPCallback(new implementation::Callback());
}
}  // namespace opentxs::network::zeromq::zap

namespace opentxs::network::zeromq::zap::implementation
{
Callback::Callback()
    : default_callback_(
          std::bind(&Callback::default_callback, this, std::placeholders::_1))
    , domains_()
    , domain_lock_()
    , policy_(Policy::Accept)
{
}

auto Callback::default_callback(const zap::Request& in) const -> OTZMQZAPReply
{
    auto output = Reply::Factory(in);

    if (Policy::Accept == policy_.load()) {
        output->SetCode(zap::Status::Success);
        output->SetStatus("OK");
    } else {
        output->SetCode(zap::Status::AuthFailure);
        output->SetStatus("Unsupported domain");
    }

    return output;
}

auto Callback::get_domain(const std::string& domain) const
    -> const Callback::Lambda&
{
    Lock lock(domain_lock_);

    try {

        return domains_.at(domain);
    } catch (...) {

        return default_callback_;
    }
}

auto Callback::Process(const zap::Request& request) const -> OTZMQZAPReply
{
    auto [valid, error] = request.Validate();

    if (false == valid) {
        LogOutput(OT_METHOD)(__func__)(": Rejecting invalid request.").Flush();

        return zap::Reply::Factory(request, Status::SystemError, error);
    }

    const auto& domain = get_domain(request.Domain());

    return domain(request);
}

auto Callback::SetDomain(
    const std::string& domain,
    const ReceiveCallback& callback) const -> bool
{
    Lock lock(domain_lock_);

    if (domain.empty()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid domain.").Flush();

        return false;
    }

    if (0 < domains_.count(domain)) {
        LogOutput(OT_METHOD)(__func__)(": Domain ")(
            domain)(" already registered.")
            .Flush();

        return false;
    }

    return domains_.emplace(domain, callback).second;
}

auto Callback::SetPolicy(const Policy policy) const -> bool
{
    policy_.store(policy);

    return true;
}
}  // namespace opentxs::network::zeromq::zap::implementation

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_ZAP_HANDLER_HPP
#define OPENTXS_NETWORK_ZEROMQ_ZAP_HANDLER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/zap/ZAP.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Handler;
}  // namespace zap
}  // namespace zeromq
}  // namespace network

using OTZMQZAPHandler = Pimpl<network::zeromq::zap::Handler>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Handler : virtual public zeromq::socket::Reply
{
public:
    OPENTXS_EXPORT static OTZMQZAPHandler Factory(
        const zeromq::Context& context,
        const Callback& callback);

    OPENTXS_EXPORT ~Handler() override = default;

protected:
    OPENTXS_EXPORT Handler() = default;

private:
    friend OTZMQZAPHandler;

#ifndef _WIN32
    Handler* clone() const noexcept override = 0;
#endif

    Handler(const Handler&) = delete;
    Handler(Handler&&) = delete;
    Handler& operator=(const Handler&) = delete;
    Handler& operator=(Handler&&) = delete;
};
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif

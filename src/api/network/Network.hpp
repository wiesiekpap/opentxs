// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "api/network/Dht.hpp"
#include "internal/api/network/Factory.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/network/Network.hpp"

namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
class Dht;
}  // namespace network

class Core;
class Endpoints;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::api::network
{
struct Network::Imp {
    const network::Asio& asio_;
    const opentxs::network::zeromq::Context& zmq_;
    std::unique_ptr<Dht> dht_;
    network::Blockchain blockchain_;

    Imp(const api::Core& api,
        const network::Asio& asio,
        const opentxs::network::zeromq::Context& zmq,
        const api::Endpoints& endpoints,
        api::network::Blockchain::Imp* blockchain,
        const bool dhtDefault,
        std::int64_t& nymPublishInterval,
        std::int64_t& nymRefreshInterval,
        std::int64_t& serverPublishInterval,
        std::int64_t& serverRefreshInterval,
        std::int64_t& unitPublishInterval,
        std::int64_t& unitRefreshInterval) noexcept
        : asio_(asio)
        , zmq_(zmq)
        , dht_(factory::DhtAPI(
              api,
              zmq_,
              endpoints,
              dhtDefault,
              nymPublishInterval,
              nymRefreshInterval,
              serverPublishInterval,
              serverRefreshInterval,
              unitPublishInterval,
              unitRefreshInterval))
        , blockchain_(blockchain)
    {
    }

private:
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::api::network

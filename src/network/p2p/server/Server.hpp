// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/api/network/Blockchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/util/Container.hpp"

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
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::p2p
{
class Server
{
public:
    using Chain = opentxs::blockchain::Type;

    auto Endpoint(const Chain chain) const noexcept -> UnallocatedCString;

    auto Disable(const Chain chain) noexcept -> void;
    auto Enable(const Chain chain) noexcept -> void;
    auto Start(
        const UnallocatedCString& sync,
        const UnallocatedCString& publicSync,
        const UnallocatedCString& update,
        const UnallocatedCString& publicUpdate) noexcept -> bool;

    Server(const api::Session& api, const zeromq::Context& zmq) noexcept;

    ~Server();

private:
    class Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::network::p2p

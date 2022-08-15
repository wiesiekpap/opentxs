// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "internal/api/network/Factory.hpp"  // IWYU pragma: associated

#include <memory>

#include "api/network/blockchain/Imp.hpp"
#include "opentxs/api/network/Blockchain.hpp"

namespace opentxs::factory
{
auto BlockchainNetworkAPI(
    const api::Session& api,
    const api::session::Endpoints& endpoints,
    const opentxs::network::zeromq::Context& zmq) noexcept
    -> api::network::Blockchain::Imp*
{
    using ReturnType = api::network::BlockchainImp;

    return std::make_unique<ReturnType>(api, endpoints, zmq).release();
}
}  // namespace opentxs::factory

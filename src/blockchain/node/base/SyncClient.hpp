// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <memory>
#include <string>

#include "opentxs/blockchain/Types.hpp"

namespace opentxs
{
namespace api
{
namespace network
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace network

class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs::blockchain::node::base
{
class SyncClient
{
public:
    auto Endpoint() const noexcept -> const std::string&;

    SyncClient(
        const api::Core& api,
        const api::network::internal::Blockchain& network,
        const Type chain) noexcept;

    ~SyncClient();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    SyncClient() = delete;
    SyncClient(const SyncClient&) = delete;
    SyncClient(SyncClient&&) = delete;
    auto operator=(const SyncClient&) -> SyncClient& = delete;
    auto operator=(SyncClient&&) -> SyncClient& = delete;
};
}  // namespace opentxs::blockchain::node::base

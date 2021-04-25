// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#ifndef OPENTXS_NETWORK_BLOCKCHAIN_SYNC_STATE_HPP
#define OPENTXS_NETWORK_BLOCKCHAIN_SYNC_STATE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace proto
{
class BlockchainP2PChainState;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace blockchain
{
namespace sync
{
class OPENTXS_EXPORT State
{
public:
    auto Chain() const noexcept -> opentxs::blockchain::Type;
    auto Position() const noexcept
        -> const opentxs::blockchain::block::Position&;
    OPENTXS_NO_EXPORT auto Serialize(
        proto::BlockchainP2PChainState& dest) const noexcept -> bool;

    OPENTXS_NO_EXPORT State(
        const api::Core& api,
        const proto::BlockchainP2PChainState& serialized) noexcept(false);
    State(
        opentxs::blockchain::Type chain,
        opentxs::blockchain::block::Position position) noexcept(false);
    State(State&&) noexcept;

    ~State();

private:
    struct Imp;

    Imp* imp_;

    State() = delete;
    State(const State&) = delete;
    auto operator=(const State&) -> State& = delete;
    auto operator=(State&&) -> State& = delete;
};
}  // namespace sync
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
#endif

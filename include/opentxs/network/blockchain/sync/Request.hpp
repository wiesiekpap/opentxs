// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_BLOCKCHAIN_SYNC_REQUEST_HPP
#define OPENTXS_NETWORK_BLOCKCHAIN_SYNC_REQUEST_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <vector>

#include "opentxs/network/blockchain/sync/Base.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"

namespace opentxs
{
namespace network
{
namespace blockchain
{
namespace sync
{
class OPENTXS_EXPORT Request final : public Base
{
public:
    using StateData = std::vector<sync::State>;

    auto State() const noexcept -> const StateData&;

    Request(StateData in) noexcept;
    OPENTXS_NO_EXPORT Request() noexcept;

    ~Request() final;

private:
    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;
};
}  // namespace sync
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
#endif

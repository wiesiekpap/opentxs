// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_BLOCKCHAIN_SYNC_ACKNOWLEDGEMENT_HPP
#define OPENTXS_NETWORK_BLOCKCHAIN_SYNC_ACKNOWLEDGEMENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>
#include <vector>

#include "opentxs/blockchain/Types.hpp"
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
class OPENTXS_EXPORT Acknowledgement final : public Base
{
public:
    using StateData = std::vector<sync::State>;

    auto Endpoint() const noexcept -> const std::string&;
    auto State() const noexcept -> const StateData&;
    /// throws std::out_of_range if specified chain is not present
    auto State(opentxs::blockchain::Type chain) const noexcept(false)
        -> const sync::State&;

    OPENTXS_NO_EXPORT Acknowledgement(
        StateData in,
        std::string endpoint) noexcept;
    OPENTXS_NO_EXPORT Acknowledgement() noexcept;

    ~Acknowledgement() final;

private:
    Acknowledgement(const Acknowledgement&) = delete;
    Acknowledgement(Acknowledgement&&) = delete;
    auto operator=(const Acknowledgement&) -> Acknowledgement& = delete;
    auto operator=(Acknowledgement&&) -> Acknowledgement& = delete;
};
}  // namespace sync
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
#endif

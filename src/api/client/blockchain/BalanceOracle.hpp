// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <memory>

#include "opentxs/blockchain/Types.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs

namespace opentxs::api::client::blockchain
{
class BalanceOracle
{
public:
    using Balance = opentxs::blockchain::Balance;
    using Chain = opentxs::blockchain::Type;

    auto RefreshBalance(const identifier::Nym& owner, const Chain chain)
        const noexcept -> void;
    auto UpdateBalance(const Chain chain, const Balance balance) const noexcept
        -> void;
    auto UpdateBalance(
        const identifier::Nym& owner,
        const Chain chain,
        const Balance balance) const noexcept -> void;

    BalanceOracle(const api::Core& api) noexcept;

    ~BalanceOracle();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    BalanceOracle() = delete;
};
}  // namespace opentxs::api::client::blockchain

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <set>
#include <tuple>
#include <utility>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

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

class Identifier;
}  // namespace opentxs

namespace opentxs::api::client::internal
{
class BalanceTreeIndex
{
public:
    using Chain = opentxs::blockchain::Type;
    using Data = std::pair<Chain, OTNymID>;

    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> std::set<OTIdentifier>;
    auto AccountList(const Chain chain) const noexcept
        -> std::set<OTIdentifier>;
    auto AccountList() const noexcept -> std::set<OTIdentifier>;
    auto Query(const Identifier& account) const noexcept -> Data;
    auto Register(
        const Identifier& account,
        const identifier::Nym& owner,
        Chain chain) const noexcept -> void;

    BalanceTreeIndex(const api::Core& api) noexcept;
    ~BalanceTreeIndex();

private:
    struct Imp;

    Imp* imp_;

    BalanceTreeIndex() = delete;
    BalanceTreeIndex(const BalanceTreeIndex&) = delete;
    BalanceTreeIndex(BalanceTreeIndex&&) = delete;
    auto operator=(const BalanceTreeIndex&) -> BalanceTreeIndex& = delete;
    auto operator=(BalanceTreeIndex&&) -> BalanceTreeIndex& = delete;
};
}  // namespace opentxs::api::client::internal

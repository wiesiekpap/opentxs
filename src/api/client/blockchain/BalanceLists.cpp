// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "api/client/Blockchain.hpp"  // IWYU pragma: associated

#include <map>

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/api/client/blockchain/Factory.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api
}  // namespace opentxs

// #define OT_METHOD
// "opentxs::api::client::implementation::Blockchain::BalanceLists::"

namespace opentxs::api::client::implementation
{
Blockchain::BalanceLists::BalanceLists(
    const api::internal::Core& api,
    api::client::internal::Blockchain& parent) noexcept
    : api_(api)
    , parent_(parent)
    , lock_()
    , lists_()
{
}

auto Blockchain::BalanceLists::Get(const Chain chain) noexcept
    -> client::blockchain::internal::BalanceList&
{
    Lock lock(lock_);
    auto it = lists_.find(chain);

    if (lists_.end() != it) { return *it->second; }

    auto [it2, added] = lists_.emplace(
        chain, factory::BlockchainBalanceList(api_, parent_, chain));

    OT_ASSERT(added);
    OT_ASSERT(it2->second);

    return *it2->second;
}
}  // namespace opentxs::api::client::implementation

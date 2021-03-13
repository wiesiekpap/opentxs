// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/client/BlockOracle.hpp"  // IWYU pragma: associated

#include <iterator>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/client/BlockOracle.hpp"

// #define OT_METHOD
// "opentxs::blockchain::client::implementation::BlockOracle::Cache::Mem::"

namespace opentxs::blockchain::client::implementation
{
BlockOracle::Cache::Mem::Mem(const std::size_t limit) noexcept
    : limit_(limit)
    , queue_()
    , index_()
{
}

auto BlockOracle::Cache::Mem::clear() noexcept -> void
{
    index_.clear();
    queue_.clear();
}

auto BlockOracle::Cache::Mem::find(const ReadView& id) const noexcept
    -> BitcoinBlockFuture
{
    if ((nullptr == id.data()) || (0 == id.size())) { return {}; }

    try {

        return index_.at(id)->second;
    } catch (...) {

        return {};
    }
}

auto BlockOracle::Cache::Mem::push(
    block::pHash&& id,
    BitcoinBlockFuture&& future) noexcept -> void
{
    if (0 == id->size()) { return; }

    const auto& item = queue_.emplace_back(std::move(id), std::move(future));
    index_[item.first->Bytes()] = queue_.crbegin();

    while (queue_.size() > limit_) {
        const auto& item = queue_.front();
        index_.erase(item.first->Bytes());
        queue_.pop_front();
    }
}
}  // namespace opentxs::blockchain::client::implementation

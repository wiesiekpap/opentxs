// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/client/blockchain/BalanceList.cpp"

#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/BalanceList.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace proto
{
class HDPath;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
class BalanceList final : virtual public internal::BalanceList
{
public:
    auto at(const std::size_t position) const noexcept(false)
        -> const_iterator::value_type& final;
    auto begin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, trees_.size());
    }
    auto Chain() const noexcept -> opentxs::blockchain::Type final
    {
        return chain_;
    }
    auto end() const noexcept -> const_iterator final
    {
        return const_iterator(this, trees_.size());
    }
    auto Parent() const noexcept -> const client::internal::Blockchain& final
    {
        return parent_;
    }
    auto size() const noexcept -> std::size_t final { return trees_.size(); }

    auto AddHDNode(
        const identifier::Nym& nym,
        const proto::HDPath& path,
        Identifier& id) noexcept -> bool final;
    auto Nym(const identifier::Nym& id) noexcept
        -> internal::BalanceTree& final;

    BalanceList(
        const api::internal::Core& api,
        const api::client::internal::Blockchain& parent,
        const opentxs::blockchain::Type chain) noexcept;

    ~BalanceList() final = default;

private:
    const client::internal::Blockchain& parent_;
    const api::internal::Core& api_;
    const opentxs::blockchain::Type chain_;
    mutable std::mutex lock_;
    std::vector<std::unique_ptr<internal::BalanceTree>> trees_;
    std::map<OTNymID, std::size_t> index_;

    using blockchain::BalanceList::at;
    auto at(const Lock& lock, const std::size_t index) const noexcept(false)
        -> const internal::BalanceTree&;
    auto factory(
        const identifier::Nym& nym,
        const std::set<OTIdentifier>& accounts) const noexcept
        -> std::unique_ptr<internal::BalanceTree>;
    using blockchain::BalanceList::size;
    auto size(const Lock& lock) const noexcept -> std::size_t;

    auto add(
        const Lock& lock,
        const identifier::Nym& id,
        std::unique_ptr<internal::BalanceTree> tree) noexcept -> bool;
    auto at(const Lock& lock, const std::size_t index) noexcept(false)
        -> internal::BalanceTree&;
    auto get_or_create(const Lock& lock, const identifier::Nym& id) noexcept
        -> internal::BalanceTree&;
    void init() noexcept;

    BalanceList(const BalanceList&) = delete;
    BalanceList(BalanceList&&) = delete;
    auto operator=(const BalanceList&) -> BalanceList& = delete;
    auto operator=(BalanceList&&) -> BalanceList& = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation

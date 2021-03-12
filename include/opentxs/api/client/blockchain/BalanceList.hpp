// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCELIST_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCELIST_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class BalanceList
{
public:
    using const_iterator =
        opentxs::iterator::Bidirectional<const BalanceList, const BalanceTree>;

    /// Throws std::out_of_range for invalid position
    OPENTXS_EXPORT virtual const_iterator::value_type& at(
        const std::size_t position) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual const_iterator begin() const noexcept = 0;
    OPENTXS_EXPORT virtual const_iterator cbegin() const noexcept = 0;
    OPENTXS_EXPORT virtual const_iterator cend() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::blockchain::Type Chain() const noexcept = 0;
    OPENTXS_EXPORT virtual const_iterator end() const noexcept = 0;
    OPENTXS_EXPORT virtual std::size_t size() const noexcept = 0;

    OPENTXS_EXPORT virtual ~BalanceList() = default;

protected:
    BalanceList() noexcept = default;

private:
    BalanceList(const BalanceList&) = delete;
    BalanceList(BalanceList&&) = delete;
    BalanceList& operator=(const BalanceList&) = delete;
    BalanceList& operator=(BalanceList&&) = delete;
};
}  // namespace blockchain
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCELIST_HPP

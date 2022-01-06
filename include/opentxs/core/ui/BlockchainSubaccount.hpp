// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/ui/List.hpp"
#include "opentxs/core/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs
{
namespace ui
{
class BlockchainSubchain;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT BlockchainSubaccount : virtual public List,
                                            virtual public ListRow
{
public:
    virtual auto First() const noexcept -> SharedPimpl<BlockchainSubchain> = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Next() const noexcept -> SharedPimpl<BlockchainSubchain> = 0;
    virtual auto SubaccountID() const noexcept -> const Identifier& = 0;

    ~BlockchainSubaccount() override = default;

protected:
    BlockchainSubaccount() noexcept = default;

private:
    BlockchainSubaccount(const BlockchainSubaccount&) = delete;
    BlockchainSubaccount(BlockchainSubaccount&&) = delete;
    auto operator=(const BlockchainSubaccount&)
        -> BlockchainSubaccount& = delete;
    auto operator=(BlockchainSubaccount&&) -> BlockchainSubaccount& = delete;
};
}  // namespace ui
}  // namespace opentxs

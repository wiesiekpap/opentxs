// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#ifndef OPENTXS_UI_BLOCKCHAINACCOUNTSTATUS_HPP
#define OPENTXS_UI_BLOCKCHAINACCOUNTSTATUS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/ui/List.hpp"

namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

namespace ui
{
class BlockchainSubaccountSource;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT BlockchainAccountStatus : virtual public List
{
public:
    virtual auto Chain() const noexcept -> blockchain::Type = 0;
    virtual auto First() const noexcept
        -> SharedPimpl<BlockchainSubaccountSource> = 0;
    virtual auto Next() const noexcept
        -> SharedPimpl<BlockchainSubaccountSource> = 0;
    virtual auto Owner() const noexcept -> const identifier::Nym& = 0;

    ~BlockchainAccountStatus() override = default;

protected:
    BlockchainAccountStatus() noexcept = default;

private:
    BlockchainAccountStatus(const BlockchainAccountStatus&) = delete;
    BlockchainAccountStatus(BlockchainAccountStatus&&) = delete;
    auto operator=(const BlockchainAccountStatus&)
        -> BlockchainAccountStatus& = delete;
    auto operator=(BlockchainAccountStatus&&)
        -> BlockchainAccountStatus& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

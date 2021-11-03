// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/ui/List.hpp"
#include "opentxs/ui/ListRow.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs
{
namespace ui
{
class BlockchainSubaccount;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT BlockchainSubaccountSource : virtual public List,
                                                  virtual public ListRow
{
public:
    virtual auto First() const noexcept
        -> SharedPimpl<BlockchainSubaccount> = 0;
    virtual auto Name() const noexcept -> std::string = 0;
    virtual auto Next() const noexcept -> SharedPimpl<BlockchainSubaccount> = 0;
    virtual auto SourceID() const noexcept -> const Identifier& = 0;
    virtual auto Type() const noexcept
        -> blockchain::crypto::SubaccountType = 0;

    ~BlockchainSubaccountSource() override = default;

protected:
    BlockchainSubaccountSource() noexcept = default;

private:
    BlockchainSubaccountSource(const BlockchainSubaccountSource&) = delete;
    BlockchainSubaccountSource(BlockchainSubaccountSource&&) = delete;
    auto operator=(const BlockchainSubaccountSource&)
        -> BlockchainSubaccountSource& = delete;
    auto operator=(BlockchainSubaccountSource&&)
        -> BlockchainSubaccountSource& = delete;
};
}  // namespace ui
}  // namespace opentxs

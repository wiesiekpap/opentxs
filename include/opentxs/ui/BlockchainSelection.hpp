// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs
{
namespace ui
{
class BlockchainSelection;
class BlockchainSelectionItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT BlockchainSelection : virtual public List
{
public:
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BlockchainSelectionItem> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BlockchainSelectionItem> = 0;

    virtual auto Disable(const blockchain::Type type) const noexcept
        -> bool = 0;
    virtual auto Enable(const blockchain::Type type) const noexcept -> bool = 0;

    ~BlockchainSelection() override = default;

protected:
    BlockchainSelection() noexcept = default;

private:
    BlockchainSelection(const BlockchainSelection&) = delete;
    BlockchainSelection(BlockchainSelection&&) = delete;
    auto operator=(const BlockchainSelection&) -> BlockchainSelection& = delete;
    auto operator=(BlockchainSelection&&) -> BlockchainSelection& = delete;
};
}  // namespace ui
}  // namespace opentxs

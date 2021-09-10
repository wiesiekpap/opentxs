// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_BLOCKCHAINSELECTIONITEM_HPP
#define OPENTXS_UI_BLOCKCHAINSELECTIONITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/ui/ListRow.hpp"

namespace opentxs
{
namespace ui
{
class BlockchainSelectionItem;
}  // namespace ui

using OTUIBlockchainSelectionItem = SharedPimpl<ui::BlockchainSelectionItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT BlockchainSelectionItem : virtual public ListRow
{
public:
    virtual auto Name() const noexcept -> std::string = 0;
    virtual auto IsEnabled() const noexcept -> bool = 0;
    virtual auto IsTestnet() const noexcept -> bool = 0;
    virtual auto Type() const noexcept -> blockchain::Type = 0;

    ~BlockchainSelectionItem() override = default;

protected:
    BlockchainSelectionItem() noexcept = default;

private:
    BlockchainSelectionItem(const BlockchainSelectionItem&) = delete;
    BlockchainSelectionItem(BlockchainSelectionItem&&) = delete;
    auto operator=(const BlockchainSelectionItem&)
        -> BlockchainSelectionItem& = delete;
    auto operator=(BlockchainSelectionItem&&)
        -> BlockchainSelectionItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_BLOCKCHAINSELECTIONITEM_HPP
#define OPENTXS_UI_BLOCKCHAINSELECTIONITEM_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/ui/ListRow.hpp"

#ifdef SWIG
// clang-format off
%extend opentxs::ui::BlockchainSelectionItem {
    int Unit() const
    {
        return static_cast<int>($self->Unit());
    }
}
%ignore opentxs::ui::BlockchainSelectionItem::Unit;
%template(OTUIBlockchainSelectionItem) opentxs::SharedPimpl<opentxs::ui::BlockchainSelectionItem>;
%rename(UIBlockchainSelectionItem) opentxs::ui::BlockchainSelectionItem;
// clang-format on
#endif  // SWIG

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
class BlockchainSelectionItem : virtual public ListRow
{
public:
    OPENTXS_EXPORT virtual std::string Name() const noexcept = 0;
    OPENTXS_EXPORT virtual bool IsEnabled() const noexcept = 0;
    OPENTXS_EXPORT virtual bool IsTestnet() const noexcept = 0;
    OPENTXS_EXPORT virtual blockchain::Type Type() const noexcept = 0;

    OPENTXS_EXPORT ~BlockchainSelectionItem() override = default;

protected:
    BlockchainSelectionItem() noexcept = default;

private:
    BlockchainSelectionItem(const BlockchainSelectionItem&) = delete;
    BlockchainSelectionItem(BlockchainSelectionItem&&) = delete;
    BlockchainSelectionItem& operator=(const BlockchainSelectionItem&) = delete;
    BlockchainSelectionItem& operator=(BlockchainSelectionItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

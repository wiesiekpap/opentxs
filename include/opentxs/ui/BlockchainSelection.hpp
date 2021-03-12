// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_BLOCKCHAINSELECTION_HPP
#define OPENTXS_UI_BLOCKCHAINSELECTION_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIBlockchainSelection) opentxs::ui::BlockchainSelection;
// clang-format on
#endif  // SWIG

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
class BlockchainSelection : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<
        opentxs::ui::BlockchainSelectionItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<
        opentxs::ui::BlockchainSelectionItem>
    Next() const noexcept = 0;

    OPENTXS_EXPORT virtual bool Disable(
        const blockchain::Type type) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Enable(
        const blockchain::Type type) const noexcept = 0;

    OPENTXS_EXPORT ~BlockchainSelection() override = default;

protected:
    BlockchainSelection() noexcept = default;

private:
    BlockchainSelection(const BlockchainSelection&) = delete;
    BlockchainSelection(BlockchainSelection&&) = delete;
    BlockchainSelection& operator=(const BlockchainSelection&) = delete;
    BlockchainSelection& operator=(BlockchainSelection&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

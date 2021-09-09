// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <list>
#include <string>

#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/base/Combined.hpp"
#include "ui/base/List.hpp"
#include "ui/base/RowType.hpp"

class QVariant;

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace ui
{
class BlockchainSubaccount;
}  // namespace ui

class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using BlockchainSubaccountList = List<
    BlockchainSubaccountExternalInterface,
    BlockchainSubaccountInternalInterface,
    BlockchainSubaccountRowID,
    BlockchainSubaccountRowInterface,
    BlockchainSubaccountRowInternal,
    BlockchainSubaccountRowBlank,
    BlockchainSubaccountSortKey,
    BlockchainSubaccountPrimaryID>;
using BlockchainSubaccountRow = RowType<
    BlockchainSubaccountSourceRowInternal,
    BlockchainSubaccountSourceInternalInterface,
    BlockchainSubaccountSourceRowID>;

class BlockchainSubaccount final : public Combined<
                                       BlockchainSubaccountList,
                                       BlockchainSubaccountRow,
                                       BlockchainSubaccountSourceSortKey>
{
public:
    auto Name() const noexcept -> std::string final { return key_; }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }
    auto SubaccountID() const noexcept -> const Identifier& final
    {
        return row_id_;
    }

    BlockchainSubaccount(
        const BlockchainSubaccountSourceInternalInterface& parent,
        const api::client::Manager& api,
        const BlockchainSubaccountSourceRowID& rowID,
        const BlockchainSubaccountSourceSortKey& key,
        CustomData& custom) noexcept;
    ~BlockchainSubaccount() final;

private:
    auto construct_row(
        const BlockchainSubaccountRowID& id,
        const BlockchainSubaccountSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto last(const BlockchainSubaccountRowID& id) const noexcept -> bool final
    {
        return BlockchainSubaccountList::last(id);
    }
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(
        const implementation::BlockchainSubaccountSourceSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;

    BlockchainSubaccount() = delete;
    BlockchainSubaccount(const BlockchainSubaccount&) = delete;
    BlockchainSubaccount(BlockchainSubaccount&&) = delete;
    auto operator=(const BlockchainSubaccount&)
        -> BlockchainSubaccount& = delete;
    auto operator=(BlockchainSubaccount&&) -> BlockchainSubaccount& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::BlockchainSubaccount>;

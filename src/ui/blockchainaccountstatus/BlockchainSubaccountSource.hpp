// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <list>
#include <string>
#include <utility>

#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/base/Combined.hpp"
#include "ui/base/List.hpp"
#include "ui/base/RowType.hpp"
#include "ui/blockchainaccountstatus/BlockchainSubaccount.hpp"

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
class BlockchainSubaccountSource;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using BlockchainSubaccountSourceList = List<
    BlockchainSubaccountSourceExternalInterface,
    BlockchainSubaccountSourceInternalInterface,
    BlockchainSubaccountSourceRowID,
    BlockchainSubaccountSourceRowInterface,
    BlockchainSubaccountSourceRowInternal,
    BlockchainSubaccountSourceRowBlank,
    BlockchainSubaccountSourceSortKey,
    BlockchainSubaccountSourcePrimaryID>;
using BlockchainSubaccountSourceRow = RowType<
    BlockchainAccountStatusRowInternal,
    BlockchainAccountStatusInternalInterface,
    BlockchainAccountStatusRowID>;

class BlockchainSubaccountSource final : public Combined<
                                             BlockchainSubaccountSourceList,
                                             BlockchainSubaccountSourceRow,
                                             BlockchainAccountStatusSortKey>
{
public:
    auto Name() const noexcept -> std::string final { return key_.second; }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }
    auto SourceID() const noexcept -> const Identifier& final
    {
        return row_id_;
    }
    auto Type() const noexcept -> blockchain::crypto::SubaccountType final
    {
        return key_.first;
    }

    BlockchainSubaccountSource(
        const BlockchainAccountStatusInternalInterface& parent,
        const api::client::Manager& api,
        const BlockchainAccountStatusRowID& rowID,
        const BlockchainAccountStatusSortKey& key,
        CustomData& custom) noexcept;
    ~BlockchainSubaccountSource() final;

private:
    auto construct_row(
        const BlockchainSubaccountSourceRowID& id,
        const BlockchainSubaccountSourceSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const BlockchainSubaccountSourceRowID& id) const noexcept
        -> bool final
    {
        return BlockchainSubaccountSourceList::last(id);
    }
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(
        const implementation::BlockchainAccountStatusSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;

    BlockchainSubaccountSource() = delete;
    BlockchainSubaccountSource(const BlockchainSubaccountSource&) = delete;
    BlockchainSubaccountSource(BlockchainSubaccountSource&&) = delete;
    auto operator=(const BlockchainSubaccountSource&)
        -> BlockchainSubaccountSource& = delete;
    auto operator=(BlockchainSubaccountSource&&)
        -> BlockchainSubaccountSource& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::BlockchainSubaccountSource>;

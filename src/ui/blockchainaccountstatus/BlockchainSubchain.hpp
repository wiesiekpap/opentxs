// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/ui/BlockchainSubchain.hpp"
#include "ui/base/Row.hpp"

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
class BlockchainSubchain;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using BlockchainSubchainRow =
    Row<BlockchainSubaccountRowInternal,
        BlockchainSubaccountInternalInterface,
        BlockchainSubaccountRowID>;

class BlockchainSubchain final : public BlockchainSubchainRow
{
public:
    auto Name() const noexcept -> std::string final;
    auto Progress() const noexcept -> std::string final;
    auto Type() const noexcept -> blockchain::crypto::Subchain final
    {
        return row_id_;
    }

    BlockchainSubchain(
        const BlockchainSubaccountInternalInterface& parent,
        const api::client::Manager& api,
        const BlockchainSubaccountRowID& rowID,
        const BlockchainSubaccountSortKey& sortKey,
        CustomData& custom) noexcept;
    ~BlockchainSubchain() final;

private:
    std::string name_;
    std::string progress_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const BlockchainSubaccountSortKey&, CustomData&) noexcept
        -> bool final;

    BlockchainSubchain() = delete;
    BlockchainSubchain(const BlockchainSubchain&) = delete;
    BlockchainSubchain(BlockchainSubchain&&) = delete;
    auto operator=(const BlockchainSubchain&) -> BlockchainSubchain& = delete;
    auto operator=(BlockchainSubchain&&) -> BlockchainSubchain& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::BlockchainSubchain>;

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <iosfwd>
#include <string>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "core/ui/base/Row.hpp"
#include "internal/core/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/ui/AccountListItem.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

class QVariant;

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class AccountListItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountListItemRow =
    Row<AccountListRowInternal, AccountListInternalInterface, AccountListRowID>;

class AccountListItem final : public AccountListItemRow
{
public:
    auto AccountID() const noexcept -> std::string final
    {
        return row_id_->str();
    }
    auto Balance() const noexcept -> Amount final;
    auto ContractID() const noexcept -> std::string final
    {
        return contract_->ID()->str();
    }
    auto DisplayBalance() const noexcept -> std::string final;
    auto DisplayUnit() const noexcept -> std::string final;
    auto Name() const noexcept -> std::string final;
    auto NotaryID() const noexcept -> std::string final
    {
        return notary_->ID()->str();
    }
    auto NotaryName() const noexcept -> std::string final
    {
        return notary_->EffectiveName();
    }
    auto Type() const noexcept -> AccountType final { return type_; }
    auto Unit() const noexcept -> core::UnitType final { return unit_; }

    AccountListItem(
        const AccountListInternalInterface& parent,
        const api::session::Client& api,
        const AccountListRowID& rowID,
        const AccountListSortKey& sortKey,
        CustomData& custom) noexcept;

    ~AccountListItem() final = default;

private:
    const AccountType type_;
    const core::UnitType unit_;
    const OTUnitDefinition contract_;
    const OTServerContract notary_;
    Amount balance_;
    std::string name_;

    static auto load_server(
        const api::Session& api,
        const identifier::Notary& id) -> OTServerContract;
    static auto load_unit(
        const api::Session& api,
        const identifier::UnitDefinition& id) -> OTUnitDefinition;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const AccountListSortKey& key, CustomData& custom) noexcept
        -> bool final;

    AccountListItem() = delete;
    AccountListItem(const AccountListItem&) = delete;
    AccountListItem(AccountListItem&&) = delete;
    auto operator=(const AccountListItem&) -> AccountListItem& = delete;
    auto operator=(AccountListItem&&) -> AccountListItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::AccountListItem>;

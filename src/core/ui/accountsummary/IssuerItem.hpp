// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "core/ui/base/Combined.hpp"
#include "core/ui/base/List.hpp"
#include "core/ui/base/RowType.hpp"
#include "core/ui/base/Widget.hpp"
#include "internal/core/ui/UI.hpp"
#include "internal/otx/client/Issuer.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
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
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

namespace ui
{
class IssuerItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
template <typename T>
struct make_blank;

template <>
struct make_blank<ui::implementation::IssuerItemRowID> {
    static auto value(const api::Session& api)
        -> ui::implementation::IssuerItemRowID
    {
        return {api.Factory().Identifier(), core::UnitType::Error};
    }
};
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using IssuerItemList = List<
    IssuerItemExternalInterface,
    IssuerItemInternalInterface,
    IssuerItemRowID,
    IssuerItemRowInterface,
    IssuerItemRowInternal,
    IssuerItemRowBlank,
    IssuerItemSortKey,
    IssuerItemPrimaryID>;
using IssuerItemRow = RowType<
    AccountSummaryRowInternal,
    AccountSummaryInternalInterface,
    AccountSummaryRowID>;

class IssuerItem final
    : public Combined<IssuerItemList, IssuerItemRow, AccountSummarySortKey>
{
public:
    auto ConnectionState() const noexcept -> bool final
    {
        return connection_.load();
    }
    auto Debug() const noexcept -> std::string final;
    auto Name() const noexcept -> std::string final;
    auto Trusted() const noexcept -> bool final { return issuer_->Paired(); }

    IssuerItem(
        const AccountSummaryInternalInterface& parent,
        const api::session::Client& api,
        const AccountSummaryRowID& rowID,
        const AccountSummarySortKey& sortKey,
        CustomData& custom,
        const core::UnitType currency) noexcept;
    ~IssuerItem() final;

private:
    const ListenerDefinitions listeners_;
    const std::string& name_;
    std::atomic<bool> connection_;
    const std::shared_ptr<const otx::client::Issuer> issuer_;
    const core::UnitType currency_;

    auto construct_row(
        const IssuerItemRowID& id,
        const IssuerItemSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto process_account(const Identifier& accountID) noexcept -> void;
    auto process_account(const Message& message) noexcept -> void;
    auto refresh_accounts() noexcept -> void;
    auto reindex(const AccountSummarySortKey& key, CustomData& custom) noexcept
        -> bool final;
    auto startup() noexcept -> void;

    IssuerItem() = delete;
    IssuerItem(const IssuerItem&) = delete;
    IssuerItem(IssuerItem&&) = delete;
    auto operator=(const IssuerItem&) -> IssuerItem& = delete;
    auto operator=(IssuerItem&&) -> IssuerItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::IssuerItem>;

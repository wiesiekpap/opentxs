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
#include "opentxs/ui/ContactListItem.hpp"
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
class ContactListItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactListItemRow =
    Row<ContactListRowInternal, ContactListInternalInterface, ContactListRowID>;

class ContactListItem : public ContactListItemRow
{
public:
    auto ContactID() const noexcept -> std::string final;
    auto DisplayName() const noexcept -> std::string final;
    auto ImageURI() const noexcept -> std::string final;
    auto Section() const noexcept -> std::string final;

    ContactListItem(
        const ContactListInternalInterface& parent,
        const api::client::Manager& api,
        const ContactListRowID& rowID,
        const ContactListSortKey& key) noexcept;
    ~ContactListItem() override = default;

protected:
    ContactListSortKey key_;

    auto translate_section(const Lock&) const noexcept -> std::string;

    using ContactListItemRow::reindex;
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void override;
    auto reindex(const ContactListSortKey&, CustomData&) noexcept
        -> bool override;
    virtual auto reindex(
        const Lock&,
        const ContactListSortKey&,
        CustomData&) noexcept -> bool;

private:
    std::string section_;

    auto calculate_section() const noexcept -> std::string;
    virtual auto calculate_section(const Lock&) const noexcept -> std::string;

    ContactListItem() = delete;
    ContactListItem(const ContactListItem&) = delete;
    ContactListItem(ContactListItem&&) = delete;
    auto operator=(const ContactListItem&) -> ContactListItem& = delete;
    auto operator=(ContactListItem&&) -> ContactListItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactListItem>;

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/ui/ContactItem.hpp"
#include "ui/base/Row.hpp"

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
class ContactItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactItemRow =
    Row<ContactSubsectionRowInternal,
        ContactSubsectionInternalInterface,
        ContactSubsectionRowID>;

class ContactItem final : public ContactItemRow
{
public:
    auto ClaimID() const noexcept -> std::string final
    {
        sLock lock(shared_lock_);

        return row_id_->str();
    }
    auto IsActive() const noexcept -> bool final
    {
        sLock lock(shared_lock_);

        return item_->isActive();
    }
    auto IsPrimary() const noexcept -> bool final
    {
        sLock lock(shared_lock_);

        return item_->isPrimary();
    }
    auto Value() const noexcept -> std::string final
    {
        sLock lock(shared_lock_);

        return item_->Value();
    }

    ContactItem(
        const ContactSubsectionInternalInterface& parent,
        const api::client::Manager& api,
        const ContactSubsectionRowID& rowID,
        const ContactSubsectionSortKey& sortKey,
        CustomData& custom) noexcept;
    ~ContactItem() final = default;

private:
    std::unique_ptr<opentxs::ContactItem> item_;

    auto reindex(
        const ContactSubsectionSortKey& key,
        CustomData& custom) noexcept -> bool final;

    ContactItem() = delete;
    ContactItem(const ContactItem&) = delete;
    ContactItem(ContactItem&&) = delete;
    auto operator=(const ContactItem&) -> ContactItem& = delete;
    auto operator=(ContactItem&&) -> ContactItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactItem>;

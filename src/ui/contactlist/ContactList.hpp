// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/ContactList.cpp"

#pragma once

#if OT_QT
#include <QAbstractItemModel>
#include <QHash>
#endif  // OT_QT
#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/util/WorkType.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal
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

class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactListList = List<
    ContactListExternalInterface,
    ContactListInternalInterface,
    ContactListRowID,
    ContactListRowInterface,
    ContactListRowInternal,
    ContactListRowBlank,
    ContactListSortKey,
    ContactListPrimaryID>;

class ContactList final : public ContactListList, Worker<ContactList>
{
public:
    auto AddContact(
        const std::string& label,
        const std::string& paymentCode,
        const std::string& nymID) const noexcept -> std::string final;
    auto ID() const noexcept -> const Identifier& final
    {
        return owner_contact_id_;
    }
#if OT_QT
    QModelIndex index(
        int row,
        int column,
        const QModelIndex& parent = QModelIndex()) const noexcept final;
#endif

    ContactList(
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;
    ~ContactList() final;

private:
    friend Worker<ContactList>;

    enum class Work : OTZMQWorkType {
        contact = value(WorkType::ContactUpdated),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = value(WorkType::Shutdown),
    };

    struct ParsedArgs {
        OTNymID nym_id_;
        OTPaymentCode payment_code_;

        ParsedArgs(
            const api::internal::Core& api,
            const std::string& purportedID,
            const std::string& purportedPaymentCode) noexcept;

    private:
        static auto extract_nymid(
            const api::internal::Core& api,
            const std::string& purportedID,
            const std::string& purportedPaymentCode) noexcept -> OTNymID;
        static auto extract_paymentcode(
            const api::internal::Core& api,
            const std::string& purportedID,
            const std::string& purportedPaymentCode) noexcept -> OTPaymentCode;
    };

    const ContactListRowID owner_contact_id_;
    std::shared_ptr<ContactListRowInternal> owner_;

    auto construct_row(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto default_id() const noexcept -> ContactListRowID final
    {
        return owner_contact_id_;
    }
    auto effective_counter(const std::size_t value) const noexcept
        -> std::size_t final
    {
        return value - 1u;
    }
#if OT_QT
    auto find_row(const ContactListRowID& id) const noexcept -> int final;
#endif
    auto first(const rLock&) const noexcept
        -> SharedPimpl<ContactListRowInterface> final;
    auto last(const ContactListRowID& id) const noexcept -> bool final;
    auto lookup(const rLock&, const ContactListRowID& id) const noexcept
        -> const ContactListRowInternal& final;

    auto add_item(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        CustomData& custom) noexcept -> void final;
    auto pipeline(const Message& in) noexcept -> void;
    auto process_contact(const Message& message) noexcept -> void;
    using ContactListList::row_modified;
    auto row_modified(const Lock&, const ContactListRowID& id) noexcept
        -> void final;
    auto startup() noexcept -> void;

    ContactList() = delete;
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    auto operator=(const ContactList&) -> ContactList& = delete;
    auto operator=(ContactList&&) -> ContactList& = delete;
};
}  // namespace opentxs::ui::implementation

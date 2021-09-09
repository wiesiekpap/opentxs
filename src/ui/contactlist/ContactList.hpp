// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <list>
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
#include "opentxs/api/Core.hpp"
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
class Manager;
}  // namespace client

class Core;
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

namespace std
{
using CONTACTLISTID = std::pair<bool, std::string>;

template <>
struct less<CONTACTLISTID> {
    auto operator()(const CONTACTLISTID& lhs, const CONTACTLISTID& rhs) const
        -> bool
    {
        const auto& [lSelf, lText] = lhs;
        const auto& [rSelf, rText] = rhs;

        if (lSelf && (!rSelf)) { return true; }

        if (rSelf && (!lSelf)) { return false; }

        if (lText < rText) { return true; }

        return false;
    }
};
}  // namespace std

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

class ContactList final : virtual public internal::ContactList,
                          public ContactListList,
                          Worker<ContactList>
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

    ContactList(
        const api::client::Manager& api,
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
            const api::Core& api,
            const std::string& purportedID,
            const std::string& purportedPaymentCode) noexcept;

    private:
        static auto extract_nymid(
            const api::Core& api,
            const std::string& purportedID,
            const std::string& purportedPaymentCode) noexcept -> OTNymID;
        static auto extract_paymentcode(
            const api::Core& api,
            const std::string& purportedID,
            const std::string& purportedPaymentCode) noexcept -> OTPaymentCode;
    };

    const ContactListRowID owner_contact_id_;

    auto construct_row(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto default_id() const noexcept -> ContactListRowID final
    {
        return owner_contact_id_;
    }

    auto pipeline(const Message& in) noexcept -> void;
    auto process_contact(const Message& message) noexcept -> void;
    auto process_contact(const Identifier& contactID) noexcept -> void;
    auto startup() noexcept -> void;

    ContactList() = delete;
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    auto operator=(const ContactList&) -> ContactList& = delete;
    auto operator=(ContactList&&) -> ContactList& = delete;
};
}  // namespace opentxs::ui::implementation

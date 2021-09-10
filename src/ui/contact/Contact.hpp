// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <utility>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/contact/ContactSectionName.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"

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

class Message;
}  // namespace zeromq
}  // namespace network

class Contact;
class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactType = List<
    ContactExternalInterface,
    ContactInternalInterface,
    ContactRowID,
    ContactRowInterface,
    ContactRowInternal,
    ContactRowBlank,
    ContactSortKey,
    ContactPrimaryID>;

class Contact final : public ContactType
{
public:
    auto ClearCallbacks() const noexcept -> void final;
    auto ContactID() const noexcept -> std::string final;
    auto DisplayName() const noexcept -> std::string final;
    auto PaymentCode() const noexcept -> std::string final;

    auto SetCallbacks(Callbacks&& cb) noexcept -> void final;

    Contact(
        const api::client::Manager& api,
        const Identifier& contactID,
        const SimpleCallback& cb) noexcept;
    ~Contact() final;

private:
    struct CallbackHolder {
        mutable std::mutex lock_{};
        Callbacks cb_{};
    };

    static const std::set<contact::ContactSectionName> allowed_types_;
    static const std::map<contact::ContactSectionName, int> sort_keys_;

    const ListenerDefinitions listeners_;
    mutable CallbackHolder callbacks_;
    std::string name_;
    std::string payment_code_;

    static auto sort_key(const contact::ContactSectionName type) noexcept
        -> int;
    static auto check_type(const contact::ContactSectionName type) noexcept
        -> bool;

    auto construct_row(
        const ContactRowID& id,
        const ContactSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const ContactRowID& id) const noexcept -> bool final
    {
        return ContactType::last(id);
    }
    auto update(ContactRowInterface& row, CustomData& custom) const noexcept
        -> void;

    auto process_contact(const opentxs::Contact& contact) noexcept -> void;
    auto process_contact(const Message& message) noexcept -> void;
    auto startup() noexcept -> void;

    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    auto operator=(const Contact&) -> Contact& = delete;
    auto operator=(Contact&&) -> Contact& = delete;
};
}  // namespace opentxs::ui::implementation

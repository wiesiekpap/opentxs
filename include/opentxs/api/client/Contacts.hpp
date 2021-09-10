// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/ContactItemType.hpp"

#ifndef OPENTXS_API_CLIENT_CONTACTS_HPP
#define OPENTXS_API_CLIENT_CONTACTS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/contact/Types.hpp"

namespace opentxs
{
namespace identity
{
class Nym;
}  // namespace identity

class Contact;
class Identifier;
class PaymentCode;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace client
{
class OPENTXS_EXPORT Contacts
{
public:
    virtual auto Contact(const Identifier& id) const
        -> std::shared_ptr<const opentxs::Contact> = 0;
    /** Returns the contact ID for a nym, if it exists */
    virtual auto ContactID(const identifier::Nym& nymID) const
        -> OTIdentifier = 0;
    virtual auto ContactList() const -> ObjectList = 0;
    virtual auto ContactName(const Identifier& contactID) const
        -> std::string = 0;
    virtual auto ContactName(
        const Identifier& contactID,
        contact::ContactItemType currencyHint) const -> std::string = 0;
    virtual auto Merge(const Identifier& parent, const Identifier& child) const
        -> std::shared_ptr<const opentxs::Contact> = 0;
    virtual auto mutable_Contact(const Identifier& id) const
        -> std::unique_ptr<Editor<opentxs::Contact>> = 0;
    virtual auto NewContact(const std::string& label) const
        -> std::shared_ptr<const opentxs::Contact> = 0;
    virtual auto NewContact(
        const std::string& label,
        const identifier::Nym& nymID,
        const PaymentCode& paymentCode) const
        -> std::shared_ptr<const opentxs::Contact> = 0;
    virtual auto NewContactFromAddress(
        const std::string& address,
        const std::string& label,
        const opentxs::blockchain::Type currency) const
        -> std::shared_ptr<const opentxs::Contact> = 0;
    /** Returns an existing contact ID if it exists, or creates a new one */
    virtual auto NymToContact(const identifier::Nym& nymID) const
        -> OTIdentifier = 0;
    /** Returns an existing contact ID if it exists, or creates a new one */
    virtual auto PaymentCodeToContact(
        const PaymentCode& code,
        const opentxs::blockchain::Type currency) const -> OTIdentifier = 0;
    virtual auto PaymentCodeToContact(
        const std::string& code,
        const opentxs::blockchain::Type currency) const -> OTIdentifier = 0;
    virtual auto Update(const identity::Nym& nym) const
        -> std::shared_ptr<const opentxs::Contact> = 0;

    OPENTXS_NO_EXPORT virtual ~Contacts() = default;

protected:
    Contacts() = default;

private:
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    auto operator=(const Contacts&) -> Contacts& = delete;
    auto operator=(Contacts&&) -> Contacts& = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif

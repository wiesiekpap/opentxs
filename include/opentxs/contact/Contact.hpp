// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONTACT_CONTACT_HPP
#define OPENTXS_CONTACT_CONTACT_HPP

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <atomic>
#include <cstdint>
#include <ctime>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"

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

class ContactData;
class ContactGroup;
class ContactItem;
class Identifier;
class PaymentCode;

namespace proto
{
class Contact;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT Contact
{
public:
    using AddressStyle = blockchain::crypto::AddressStyle;
    using BlockchainType = blockchain::Type;
    using BlockchainAddress = std::tuple<OTData, AddressStyle, BlockchainType>;

    static auto Best(const ContactGroup& group) -> std::shared_ptr<ContactItem>;
    static auto ExtractLabel(const identity::Nym& nym) -> std::string;
    static auto ExtractType(const identity::Nym& nym)
        -> contact::ContactItemType;
    static auto PaymentCode(
        const ContactData& data,
        const contact::ContactItemType currency) -> std::string;

    OPENTXS_NO_EXPORT Contact(
        const api::client::Manager& api,
        const proto::Contact& serialized);
    Contact(const api::client::Manager& api, const std::string& label);

    auto operator+=(Contact& rhs) -> Contact&;

    auto BestEmail() const -> std::string;
    auto BestPhoneNumber() const -> std::string;
    auto BestSocialMediaProfile(const contact::ContactItemType type) const
        -> std::string;
    auto BlockchainAddresses() const -> std::vector<BlockchainAddress>;
    auto Data() const -> std::shared_ptr<ContactData>;
    auto EmailAddresses(bool active = true) const -> std::string;
    auto ID() const -> const Identifier&;
    auto Label() const -> const std::string&;
    auto LastUpdated() const -> std::time_t;
    auto Nyms(const bool includeInactive = false) const -> std::vector<OTNymID>;
    auto PaymentCode(
        const contact::ContactItemType currency =
            contact::ContactItemType::BTC) const -> std::string;
    auto PaymentCodes(
        const contact::ContactItemType currency =
            contact::ContactItemType::BTC) const -> std::vector<std::string>;
    auto PhoneNumbers(bool active = true) const -> std::string;
    auto Print() const -> std::string;
    OPENTXS_NO_EXPORT auto Serialize(proto::Contact& out) const -> bool;
    auto SocialMediaProfiles(
        const contact::ContactItemType type,
        bool active = true) const -> std::string;
    auto SocialMediaProfileTypes() const
        -> const std::set<contact::ContactItemType>;
    auto Type() const -> contact::ContactItemType;

    auto AddBlockchainAddress(
        const std::string& address,
        const BlockchainType currency) -> bool;
    auto AddBlockchainAddress(
        const blockchain::crypto::AddressStyle& style,
        const blockchain::Type chain,
        const opentxs::Data& bytes) -> bool;
    auto AddEmail(
        const std::string& value,
        const bool primary,
        const bool active) -> bool;
    auto AddNym(const Nym_p& nym, const bool primary) -> bool;
    auto AddNym(const identifier::Nym& nymID, const bool primary) -> bool;
    auto AddPaymentCode(
        const opentxs::PaymentCode& code,
        const bool primary,
        const contact::ContactItemType currency = contact::ContactItemType::BTC,
        const bool active = true) -> bool;
    auto AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active) -> bool;
    auto AddSocialMediaProfile(
        const std::string& value,
        const contact::ContactItemType type,
        const bool primary,
        const bool active) -> bool;
    auto RemoveNym(const identifier::Nym& nymID) -> bool;
    void SetLabel(const std::string& label);
    void Update(const identity::Nym::Serialized& nym);

    ~Contact();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    auto operator=(const Contact&) -> Contact& = delete;
    auto operator=(Contact&&) -> Contact& = delete;
};
}  // namespace opentxs
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONTACT_CONTACT_HPP
#define OPENTXS_CONTACT_CONTACT_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"
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

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/AddressStyle.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/protobuf/Contact.pb.h"
#include "opentxs/protobuf/ContactEnums.pb.h"

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

class ContactGroup;
class ContactItem;
class PaymentCode;
}  // namespace opentxs

namespace opentxs
{
class Contact
{
public:
    using AddressStyle = api::client::blockchain::AddressStyle;
    using BlockchainType = blockchain::Type;
    using BlockchainAddress = std::tuple<OTData, AddressStyle, BlockchainType>;

    OPENTXS_EXPORT static std::shared_ptr<ContactItem> Best(
        const ContactGroup& group);
    OPENTXS_EXPORT static std::string ExtractLabel(const identity::Nym& nym);
    OPENTXS_EXPORT static proto::ContactItemType ExtractType(
        const identity::Nym& nym);
    OPENTXS_EXPORT static std::string PaymentCode(
        const ContactData& data,
        const proto::ContactItemType currency);

    OPENTXS_EXPORT Contact(
        const api::client::internal::Manager& api,
        const proto::Contact& serialized);
    OPENTXS_EXPORT Contact(
        const api::client::internal::Manager& api,
        const std::string& label);

    OPENTXS_EXPORT operator proto::Contact() const;
    OPENTXS_EXPORT Contact& operator+=(Contact& rhs);

    OPENTXS_EXPORT std::string BestEmail() const;
    OPENTXS_EXPORT std::string BestPhoneNumber() const;
    OPENTXS_EXPORT std::string BestSocialMediaProfile(
        const proto::ContactItemType type) const;
    OPENTXS_EXPORT std::vector<BlockchainAddress> BlockchainAddresses() const;
    OPENTXS_EXPORT std::shared_ptr<ContactData> Data() const;
    OPENTXS_EXPORT std::string EmailAddresses(bool active = true) const;
    OPENTXS_EXPORT const Identifier& ID() const;
    OPENTXS_EXPORT const std::string& Label() const;
    OPENTXS_EXPORT std::time_t LastUpdated() const;
    OPENTXS_EXPORT std::vector<OTNymID> Nyms(
        const bool includeInactive = false) const;
    OPENTXS_EXPORT std::string PaymentCode(
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const;
    OPENTXS_EXPORT std::vector<std::string> PaymentCodes(
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const;
    OPENTXS_EXPORT std::string PhoneNumbers(bool active = true) const;
    OPENTXS_EXPORT std::string Print() const;
    OPENTXS_EXPORT std::string SocialMediaProfiles(
        const proto::ContactItemType type,
        bool active = true) const;
    OPENTXS_EXPORT const std::set<proto::ContactItemType>
    SocialMediaProfileTypes() const;
    OPENTXS_EXPORT proto::ContactItemType Type() const;

    OPENTXS_EXPORT bool AddBlockchainAddress(
        const std::string& address,
        const proto::ContactItemType currency = proto::CITEMTYPE_UNKNOWN);
    OPENTXS_EXPORT bool AddBlockchainAddress(
        const api::client::blockchain::AddressStyle& style,
        const blockchain::Type chain,
        const opentxs::Data& bytes);
    OPENTXS_EXPORT bool AddEmail(
        const std::string& value,
        const bool primary,
        const bool active);
    OPENTXS_EXPORT bool AddNym(const Nym_p& nym, const bool primary);
    OPENTXS_EXPORT bool AddNym(
        const identifier::Nym& nymID,
        const bool primary);
    OPENTXS_EXPORT bool AddPaymentCode(
        const opentxs::PaymentCode& code,
        const bool primary,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC,
        const bool active = true);
    OPENTXS_EXPORT bool AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active);
    OPENTXS_EXPORT bool AddSocialMediaProfile(
        const std::string& value,
        const proto::ContactItemType type,
        const bool primary,
        const bool active);
    OPENTXS_EXPORT bool RemoveNym(const identifier::Nym& nymID);
    OPENTXS_EXPORT void SetLabel(const std::string& label);
    OPENTXS_EXPORT void Update(const identity::Nym::Serialized& nym);

    OPENTXS_EXPORT ~Contact() = default;

private:
    const api::client::internal::Manager& api_;
    VersionNumber version_{0};
    std::string label_{""};
    mutable std::mutex lock_{};
    const OTIdentifier id_;
    OTIdentifier parent_;
    OTNymID primary_nym_;
    std::map<OTNymID, Nym_p> nyms_;
    std::set<OTIdentifier> merged_children_;
    std::unique_ptr<ContactData> contact_data_{};
    mutable std::shared_ptr<ContactData> cached_contact_data_{};
    std::atomic<std::uint64_t> revision_{0};

    static VersionNumber check_version(
        const VersionNumber in,
        const VersionNumber targetVersion);
    static OTIdentifier generate_id(const api::internal::Core& api);
    static BlockchainAddress translate(
        const api::client::internal::Manager& api,
        const proto::ContactItemType chain,
        const std::string& value,
        const std::string& subtype) noexcept(false);

    std::shared_ptr<ContactGroup> payment_codes(
        const Lock& lock,
        const proto::ContactItemType currency) const;
    std::shared_ptr<ContactData> merged_data(const Lock& lock) const;
    proto::ContactItemType type(const Lock& lock) const;
    bool verify_write_lock(const Lock& lock) const;

    bool add_nym(const Lock& lock, const Nym_p& nym, const bool primary);
    bool add_claim(const std::shared_ptr<ContactItem>& item);
    bool add_claim(const Lock& lock, const std::shared_ptr<ContactItem>& item);
    void add_nym_claim(
        const Lock& lock,
        const identifier::Nym& nymID,
        const bool primary);
    void add_verified_claim(
        const Lock& lock,
        const std::shared_ptr<ContactItem>& item);
    void init_nyms();
    void update_label(const Lock& lock, const identity::Nym& nym);

    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace opentxs
#endif

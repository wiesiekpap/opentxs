// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_NYMDATA_HPP
#define OPENTXS_CLIENT_NYMDATA_HPP

// IWYU pragma: no_include "opentxs/contact/ContactItemType.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <set>
#include <string>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/identity/Nym.hpp"

namespace opentxs
{
namespace api
{
class Factory;

namespace implementation
{
class Wallet;
}  // namespace implementation
}  // namespace api

namespace identifier
{
class UnitDefinition;
}  // namespace identifier

namespace proto
{
class ContactData;
}  // namespace proto

class ContactData;
class Identifier;
class NymParameters;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT NymData
{
public:
    NymData(const NymData&);
    NymData(NymData&&);

    auto asPublicNym() const -> identity::Nym::Serialized;
    auto BestEmail() const -> std::string;
    auto BestPhoneNumber() const -> std::string;
    auto BestSocialMediaProfile(const contact::ContactItemType type) const
        -> std::string;
    auto Claims() const -> const opentxs::ContactData&;
    auto DeleteClaim(const Identifier& id, const PasswordPrompt& reason)
        -> bool;
    auto EmailAddresses(bool active = true) const -> std::string;
    auto HaveContract(
        const identifier::UnitDefinition& id,
        const contact::ContactItemType currency,
        const bool primary,
        const bool active) const -> bool;
    auto Name() const -> std::string;
    auto Nym() const -> const identity::Nym&;
    auto PaymentCode(const contact::ContactItemType currency) const
        -> std::string;
    auto PhoneNumbers(bool active = true) const -> std::string;
    auto PreferredOTServer() const -> std::string;
    auto PrintContactData() const -> std::string;
    auto SocialMediaProfiles(
        const contact::ContactItemType type,
        bool active = true) const -> std::string;
    auto SocialMediaProfileTypes() const -> std::set<contact::ContactItemType>;
    auto Type() const -> contact::ContactItemType;
    auto Valid() const -> bool;

    auto AddChildKeyCredential(
        const Identifier& strMasterID,
        const NymParameters& nymParameters,
        const PasswordPrompt& reason) -> std::string;
    auto AddClaim(const Claim& claim, const PasswordPrompt& reason) -> bool;
    auto AddContract(
        const std::string& instrumentDefinitionID,
        const contact::ContactItemType currency,
        const bool primary,
        const bool active,
        const PasswordPrompt& reason) -> bool;
    auto AddEmail(
        const std::string& value,
        const bool primary,
        const bool active,
        const PasswordPrompt& reason) -> bool;
    auto AddPaymentCode(
        const std::string& code,
        const contact::ContactItemType currency,
        const bool primary,
        const bool active,
        const PasswordPrompt& reason) -> bool;
    auto AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active,
        const PasswordPrompt& reason) -> bool;
    auto AddPreferredOTServer(
        const std::string& id,
        const bool primary,
        const PasswordPrompt& reason) -> bool;
    auto AddSocialMediaProfile(
        const std::string& value,
        const contact::ContactItemType type,
        const bool primary,
        const bool active,
        const PasswordPrompt& reason) -> bool;
    void Release();
    auto SetCommonName(const std::string& name, const PasswordPrompt& reason)
        -> bool;
    OPENTXS_NO_EXPORT auto SetContactData(
        const proto::ContactData& data,
        const PasswordPrompt& reason) -> bool;
    auto SetContactData(const ReadView& data, const PasswordPrompt& reason)
        -> bool;
    auto SetScope(
        const contact::ContactItemType type,
        const std::string& name,
        const bool primary,
        const PasswordPrompt& reason) -> bool;

    ~NymData();

private:
    friend api::implementation::Wallet;

    using Lock = std::unique_lock<std::mutex>;
    using LockedSave = std::function<void(NymData*, Lock&)>;

    const api::Factory& factory_;
    std::unique_ptr<Lock> object_lock_;
    std::unique_ptr<LockedSave> locked_save_callback_;

    std::shared_ptr<identity::Nym> nym_;

    auto data() const -> const ContactData&;

    auto nym() const -> const identity::Nym&;
    auto nym() -> identity::Nym&;

    void release();

    NymData(
        const api::Factory& factory,
        std::mutex& objectMutex,
        const std::shared_ptr<identity::Nym>& nym,
        LockedSave save);
    NymData() = delete;
    auto operator=(const NymData&) -> NymData& = delete;
    auto operator=(NymData&&) -> NymData& = delete;
};
}  // namespace opentxs
#endif

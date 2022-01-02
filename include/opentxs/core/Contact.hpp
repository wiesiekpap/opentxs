// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/identity/wot/claim/ClaimType.hpp"

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
#include "opentxs/core/Data.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace identity
{
namespace wot
{
namespace claim
{
class Data;
class Group;
class Item;
}  // namespace claim
}  // namespace wot
}  // namespace identity

namespace proto
{
class Contact;
class Nym;
}  // namespace proto

class Identifier;
class PaymentCode;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT Contact
{
public:
    static auto Best(const identity::wot::claim::Group& group)
        -> std::shared_ptr<identity::wot::claim::Item>;
    static auto ExtractLabel(const identity::Nym& nym) -> std::string;
    static auto ExtractType(const identity::Nym& nym)
        -> identity::wot::claim::ClaimType;
    static auto PaymentCode(
        const identity::wot::claim::Data& data,
        const core::UnitType currency) -> std::string;

    OPENTXS_NO_EXPORT Contact(
        const api::session::Client& api,
        const proto::Contact& serialized);
    Contact(const api::session::Client& api, const std::string& label);

    auto operator+=(Contact& rhs) -> Contact&;

    auto BestEmail() const -> std::string;
    auto BestPhoneNumber() const -> std::string;
    auto BestSocialMediaProfile(
        const identity::wot::claim::ClaimType type) const -> std::string;
    auto BlockchainAddresses() const -> std::vector<
        std::tuple<OTData, blockchain::crypto::AddressStyle, blockchain::Type>>;
    auto Data() const -> std::shared_ptr<identity::wot::claim::Data>;
    auto EmailAddresses(bool active = true) const -> std::string;
    auto ID() const -> const Identifier&;
    auto Label() const -> const std::string&;
    auto LastUpdated() const -> std::time_t;
    auto Nyms(const bool includeInactive = false) const -> std::vector<OTNymID>;
    auto PaymentCode(const core::UnitType currency = core::UnitType::BTC) const
        -> std::string;
    auto PaymentCodes(const core::UnitType currency = core::UnitType::BTC) const
        -> std::vector<std::string>;
    auto PhoneNumbers(bool active = true) const -> std::string;
    auto Print() const -> std::string;
    OPENTXS_NO_EXPORT auto Serialize(proto::Contact& out) const -> bool;
    auto SocialMediaProfiles(
        const identity::wot::claim::ClaimType type,
        bool active = true) const -> std::string;
    auto SocialMediaProfileTypes() const
        -> const std::set<identity::wot::claim::ClaimType>;
    auto Type() const -> identity::wot::claim::ClaimType;

    auto AddBlockchainAddress(
        const std::string& address,
        const blockchain::Type currency) -> bool;
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
        const core::UnitType currency = core::UnitType::BTC,
        const bool active = true) -> bool;
    auto AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active) -> bool;
    auto AddSocialMediaProfile(
        const std::string& value,
        const identity::wot::claim::ClaimType type,
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

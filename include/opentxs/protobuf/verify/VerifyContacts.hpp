// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFYCONTACTS_HPP
#define OPENTXS_PROTOBUF_VERIFYCONTACTS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/Contact.hpp"

namespace opentxs
{
namespace proto
{
enum class ClaimType : bool {
    Indexed = true,
    Normal = false,
};
enum class VerificationType : bool {
    Indexed = true,
    Normal = false,
};

OPENTXS_EXPORT const VersionMap& ContactAllowedContactData() noexcept;
OPENTXS_EXPORT const VersionMap& ContactDataAllowedContactSection() noexcept;
OPENTXS_EXPORT const VersionMap& ContactSectionAllowedItem() noexcept;
OPENTXS_EXPORT const VersionMap& VerificationAllowedSignature() noexcept;
OPENTXS_EXPORT const VersionMap& VerificationGroupAllowedIdentity() noexcept;
OPENTXS_EXPORT const VersionMap&
VerificationIdentityAllowedVerification() noexcept;
OPENTXS_EXPORT const VersionMap& VerificationOfferAllowedClaim() noexcept;
OPENTXS_EXPORT const VersionMap&
VerificationOfferAllowedVerification() noexcept;
OPENTXS_EXPORT const VersionMap& VerificationSetAllowedGroup() noexcept;

OPENTXS_EXPORT bool ValidContactSectionName(
    const std::uint32_t version,
    const ContactSectionName name);
OPENTXS_EXPORT bool ValidContactItemType(
    const ContactSectionVersion version,
    const ContactItemType itemType);
OPENTXS_EXPORT bool ValidContactItemAttribute(
    const std::uint32_t version,
    const ContactItemAttribute attribute);

OPENTXS_EXPORT std::string TranslateSectionName(
    const std::uint32_t enumValue,
    const std::string& lang = "en");
OPENTXS_EXPORT std::string TranslateItemType(
    const std::uint32_t enumValue,
    const std::string& lang = "en");
OPENTXS_EXPORT std::string TranslateItemAttributes(
    const std::uint32_t enumValue,
    const std::string& lang = "en");
OPENTXS_EXPORT std::uint32_t ReciprocalRelationship(
    const std::uint32_t relationship);
OPENTXS_EXPORT bool CheckCombination(
    const ContactSectionName section,
    const ContactItemType type,
    const std::uint32_t version = 1);
OPENTXS_EXPORT std::uint32_t RequiredVersion(
    const ContactSectionName section,
    const ContactItemType type,
    const std::uint32_t hint = 1);
OPENTXS_EXPORT std::uint32_t NymRequiredVersion(
    const std::uint32_t contactDataVersion,
    const std::uint32_t hint);
OPENTXS_EXPORT std::uint32_t RequiredAuthorityVersion(
    const std::uint32_t contactDataVersion,
    const std::uint32_t hint);
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYCONTACTS_HPP

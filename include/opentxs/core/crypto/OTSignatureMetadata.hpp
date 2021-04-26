// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTSIGNATUREMETADATA_HPP
#define OPENTXS_CORE_CRYPTO_OTSIGNATUREMETADATA_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class OPENTXS_EXPORT OTSignatureMetadata
{
public:
    bool operator==(const OTSignatureMetadata& rhs) const;

    bool operator!=(const OTSignatureMetadata& rhs) const
    {
        return !(operator==(rhs));
    }

    bool SetMetadata(
        char metaKeyType,
        char metaNymID,
        char metaMasterCredID,
        char metaChildCredID);

    inline bool HasMetadata() const { return hasMetadata_; }

    inline char GetKeyType() const { return metaKeyType_; }

    inline char FirstCharNymID() const { return metaNymID_; }

    inline char FirstCharMasterCredID() const { return metaMasterCredID_; }

    inline char FirstCharChildCredID() const { return metaChildCredID_; }

    OTSignatureMetadata(const api::internal::Core& api);
    OTSignatureMetadata& operator=(const OTSignatureMetadata& rhs);

private:
    const api::internal::Core& api_;
    // Defaults to false. Is set true by calling SetMetadata
    bool hasMetadata_{false};
    // Can be A, E, or S (authentication, encryption, or signing.
    // Also, E would be unusual.)
    char metaKeyType_{0x0};
    // Can be any letter from base62 alphabet. Represents
    // first letter of a Nym's ID.
    char metaNymID_{0x0};
    // Can be any letter from base62 alphabet.
    // Represents first letter of a Master Credential
    // ID (for that Nym.)
    char metaMasterCredID_{0x0};
    // Can be any letter from base62 alphabet. Represents
    // first letter of a Credential ID (signed by that Master.)
    char metaChildCredID_{0x0};
};
}  // namespace opentxs
#endif

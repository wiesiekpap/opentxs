// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_BIP32_HPP
#define OPENTXS_CRYPTO_BIP32_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
class HD;
}  // namespace key
}  // namespace crypto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
namespace crypto
{
std::string Print(const proto::HDPath& node);

class Bip32
{
public:
    using Path = std::vector<Bip32Index>;
    using Key = std::tuple<OTSecret, OTSecret, OTData, Path, Bip32Fingerprint>;

#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual Key DeriveKey(
        const EcdsaCurve& curve,
        const Secret& seed,
        const Path& path) const = 0;
    /// throws std::runtime_error on invalid inputs
    OPENTXS_EXPORT virtual Key DerivePrivateKey(
        const key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) = 0;
    /// throws std::runtime_error on invalid inputs
    OPENTXS_EXPORT virtual Key DerivePublicKey(
        const key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) = 0;
#endif  // OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual bool DeserializePrivate(
        const std::string& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Secret& key) const = 0;
    OPENTXS_EXPORT virtual bool DeserializePublic(
        const std::string& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Data& key) const = 0;
    OPENTXS_EXPORT virtual OTIdentifier SeedID(
        const ReadView entropy) const = 0;
    OPENTXS_EXPORT virtual std::string SerializePrivate(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Secret& key) const = 0;
    OPENTXS_EXPORT virtual std::string SerializePublic(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Data& key) const = 0;

    OPENTXS_EXPORT virtual ~Bip32() = default;
};
}  // namespace crypto
}  // namespace opentxs
#endif

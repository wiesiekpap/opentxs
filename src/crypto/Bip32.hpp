// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <string>

#include "HDNode.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace crypto
{
namespace key
{
class HD;
}  // namespace key

class EcdsaProvider;
}  // namespace crypto

class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace be = boost::endian;

namespace opentxs::crypto::implementation
{
class Bip32 : virtual public opentxs::crypto::Bip32
{
public:
#if OT_CRYPTO_WITH_BIP32
    auto DeriveKey(
        const EcdsaCurve& curve,
        const Secret& seed,
        const Path& path) const -> Key final;
    auto DerivePrivateKey(
        const key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key final;
    auto DerivePublicKey(
        const key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto DeserializePrivate(
        const std::string& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Secret& key) const -> bool final;
    auto DeserializePublic(
        const std::string& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Data& key) const -> bool final;
    auto SeedID(const ReadView entropy) const -> OTIdentifier final;
    auto SerializePrivate(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Secret& key) const -> std::string final;
    auto SerializePublic(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Data& key) const -> std::string final;

    Bip32(const api::Crypto& crypto) noexcept;

private:
    const api::Crypto& crypto_;

    static auto IsHard(const Bip32Index) noexcept -> bool;

    auto ckd_hardened(
        const HDNode& node,
        const be::big_uint32_buf_t i,
        const WritableView& data) const noexcept -> void;
    auto ckd_normal(
        const HDNode& node,
        const be::big_uint32_buf_t i,
        const WritableView& data) const noexcept -> void;
    auto decode(const std::string& serialized) const noexcept -> OTData;
    auto derive_private(
        HDNode& node,
        Bip32Fingerprint& parent,
        const Bip32Index child) const noexcept -> bool;
    auto derive_public(
        HDNode& node,
        Bip32Fingerprint& parent,
        const Bip32Index child) const noexcept -> bool;
    auto extract(
        const Data& input,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode) const noexcept -> bool;
    auto provider(const EcdsaCurve& curve) const noexcept(false)
        -> const crypto::EcdsaProvider&;
#if OT_CRYPTO_WITH_BIP32
    auto root_node(
        const EcdsaCurve& curve,
        const ReadView entropy,
        const AllocateOutput privateKey,
        const AllocateOutput code,
        const AllocateOutput publicKey) const noexcept -> bool;
#endif  // OT_CRYPTO_WITH_BIP32

    Bip32() = delete;
    Bip32(const Bip32&) = delete;
    Bip32(Bip32&&) = delete;
    auto operator=(const Bip32&) -> Bip32& = delete;
    auto operator=(Bip32&&) -> Bip32& = delete;
};
}  // namespace opentxs::crypto::implementation

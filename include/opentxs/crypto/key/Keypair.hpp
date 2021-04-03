// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_KEYPAIR_HPP
#define OPENTXS_CRYPTO_KEY_KEYPAIR_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <list>
#include <memory>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
class Asymmetric;
class Keypair;
}  // namespace key
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
}  // namespace proto

class Data;
class PasswordPrompt;
class Secret;
class Signature;

using OTKeypair = Pimpl<crypto::key::Keypair>;
}  // namespace opentxs

namespace opentxs
{
namespace crypto
{
namespace key
{
class Keypair
{
public:
    using Keys = std::list<const Asymmetric*>;

    OPENTXS_EXPORT virtual operator bool() const noexcept = 0;

    OPENTXS_EXPORT virtual bool CheckCapability(
        const NymCapability& capability) const noexcept = 0;
    /// throws std::runtime_error if private key is missing
    OPENTXS_EXPORT virtual const Asymmetric& GetPrivateKey() const
        noexcept(false) = 0;
    /// throws std::runtime_error if public key is missing
    OPENTXS_EXPORT virtual const Asymmetric& GetPublicKey() const
        noexcept(false) = 0;
    // inclusive means, return keys when theSignature has no metadata.
    OPENTXS_EXPORT virtual std::int32_t GetPublicKeyBySignature(
        Keys& listOutput,
        const Signature& theSignature,
        bool bInclusive = false) const noexcept = 0;
    OPENTXS_EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetSerialized(
        bool privateKey) const noexcept = 0;
    OPENTXS_EXPORT virtual bool GetTransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const noexcept = 0;

    OPENTXS_EXPORT virtual ~Keypair() = default;

protected:
    Keypair() = default;

private:
    friend OTKeypair;

    virtual Keypair* clone() const = 0;

    Keypair(const Keypair&) = delete;
    Keypair(Keypair&&) = delete;
    Keypair& operator=(const Keypair&) = delete;
    Keypair& operator=(Keypair&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif

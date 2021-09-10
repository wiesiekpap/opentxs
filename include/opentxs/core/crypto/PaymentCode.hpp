// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP
#define OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
class Asymmetric;
class EllipticCurve;
class HD;
}  // namespace key
}  // namespace crypto

namespace identity
{
namespace credential
{
class Base;
}  // namespace credential
}  // namespace identity

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class Credential;
class PaymentCode;
class Signature;
}  // namespace proto

class Data;
class PaymentCode;
class PasswordPrompt;

using OTPaymentCode = Pimpl<PaymentCode>;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT PaymentCode
{
public:
    using ECKey = std::unique_ptr<crypto::key::EllipticCurve>;
    using HDKey = std::shared_ptr<crypto::key::HD>;
    using Serialized = proto::PaymentCode;
    using Elements = std::vector<Space>;

    static const VersionNumber DefaultVersion;

    virtual operator const crypto::key::Asymmetric&() const noexcept = 0;

    OPENTXS_NO_EXPORT virtual auto operator==(
        const Serialized& rhs) const noexcept -> bool = 0;

    virtual auto asBase58() const noexcept -> std::string = 0;
    virtual auto Blind(
        const PaymentCode& recipient,
        const crypto::key::EllipticCurve& privateKey,
        const ReadView outpoint,
        const AllocateOutput destination,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto BlindV3(
        const PaymentCode& recipient,
        const crypto::key::EllipticCurve& privateKey,
        const AllocateOutput destination,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto DecodeNotificationElements(
        const std::uint8_t version,
        const Elements& elements,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<PaymentCode> = 0;
    virtual auto GenerateNotificationElements(
        const PaymentCode& recipient,
        const crypto::key::EllipticCurve& privateKey,
        const PasswordPrompt& reason) const noexcept -> Elements = 0;
    virtual auto ID() const noexcept -> const identifier::Nym& = 0;
    virtual auto Key() const noexcept -> HDKey = 0;
    virtual auto Incoming(
        const PaymentCode& sender,
        const Bip32Index index,
        const blockchain::Type chain,
        const PasswordPrompt& reason,
        const std::uint8_t version = 0) const noexcept -> ECKey = 0;
    virtual auto Locator(
        const AllocateOutput destination,
        const std::uint8_t version = 0) const noexcept -> bool = 0;
    virtual auto Outgoing(
        const PaymentCode& recipient,
        const Bip32Index index,
        const blockchain::Type chain,
        const PasswordPrompt& reason,
        const std::uint8_t version = 0) const noexcept -> ECKey = 0;
    virtual auto Serialize(AllocateOutput destination) const noexcept
        -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        Serialized& serialized) const noexcept -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Sign(
        const identity::credential::Base& credential,
        proto::Signature& sig,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto Sign(
        const Data& data,
        Data& output,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto Unblind(
        const ReadView blinded,
        const crypto::key::EllipticCurve& publicKey,
        const ReadView outpoint,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<PaymentCode> = 0;
    virtual auto UnblindV3(
        const std::uint8_t version,
        const ReadView blinded,
        const crypto::key::EllipticCurve& publicKey,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<PaymentCode> = 0;
    virtual auto Valid() const noexcept -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const noexcept -> bool = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    virtual auto AddPrivateKeys(
        std::string& seed,
        const Bip32Index index,
        const PasswordPrompt& reason) noexcept -> bool = 0;

    virtual ~PaymentCode() = default;

protected:
    PaymentCode() = default;

private:
    friend OTPaymentCode;

    virtual auto clone() const -> PaymentCode* = 0;

    PaymentCode(const PaymentCode&) = delete;
    PaymentCode(PaymentCode&&) = delete;
    auto operator=(const PaymentCode&) -> PaymentCode&;
    auto operator=(PaymentCode&&) -> PaymentCode&;
};
}  // namespace opentxs
#endif

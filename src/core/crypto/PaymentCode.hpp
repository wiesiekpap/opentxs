// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/core/crypto/PaymentCode.cpp"

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace crypto
{
namespace key
{
class Secp256k1;
}  // namespace key
}  // namespace crypto

namespace identity
{
namespace credential
{
class Base;
}  // namespace credential
}  // namespace identity

namespace proto
{
class Credential;
class PaymentCode;
class Signature;
}  // namespace proto

class Factory;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::implementation
{
class PaymentCode final : virtual public opentxs::PaymentCode
{
public:
    struct SerializedForBase58 {
        std::uint8_t prefix_;
        std::uint8_t version_;
        std::uint8_t features_;
        std::array<char, 33> key_;
        std::array<char, 32> code_;
        std::uint8_t bm_version_;
        std::uint8_t bm_stream_;
        std::array<std::byte, 11> blank_;

        operator ReadView() const noexcept
        {
            return {reinterpret_cast<const char*>(this), sizeof(*this)};
        }

        auto haveBitmessage() const noexcept
        {
            return 0 != (features_ & std::uint8_t{0x80});
        }

        SerializedForBase58() noexcept
            : prefix_(0)
            , version_(0)
            , features_(0)
            , key_()
            , code_()
            , bm_version_(0)
            , bm_stream_(0)
            , blank_()
        {
            static_assert(81 == sizeof(SerializedForBase58));
        }

        SerializedForBase58(
            const VersionNumber version,
            const bool bitmessage,
            const std::uint8_t bmVersion,
            const std::uint8_t bmStream) noexcept
            : prefix_(0x47)
            , version_(static_cast<std::uint8_t>(version))
            , features_(bitmessage ? 0x80 : 0x00)
            , key_()
            , code_()
            , bm_version_(bmVersion)
            , bm_stream_(bmStream)
            , blank_()
        {
            static_assert(81 == sizeof(SerializedForBase58));
        }
    };

    static const std::size_t pubkey_size_;
    static const std::size_t chain_code_size_;

    operator const opentxs::crypto::key::Asymmetric&() const noexcept final;

    auto operator==(const proto::PaymentCode& rhs) const noexcept -> bool final;

    auto ID() const noexcept -> const identifier::Nym& final { return id_; }
    auto asBase58() const noexcept -> std::string final;
    auto Serialize() const noexcept -> Serialized final;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto Sign(
        const identity::credential::Base& credential,
        proto::Signature& sig,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto Sign(const Data& data, Data& output, const PasswordPrompt& reason)
        const noexcept -> bool final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto Valid() const noexcept -> bool final;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const noexcept -> bool final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto Version() const noexcept -> VersionNumber final { return version_; }

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    auto AddPrivateKeys(
        std::string& seed,
        const Bip32Index index,
        const PasswordPrompt& reason) noexcept -> bool final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

    PaymentCode(
        const api::internal::Core& api,
        const std::uint8_t version,
        const bool hasBitmessage,
        const ReadView pubkey,
        const ReadView chaincode,
        const std::uint8_t bitmessageVersion,
        const std::uint8_t bitmessageStream
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        ,
        std::unique_ptr<crypto::key::Secp256k1> key
#endif
        ) noexcept;

    ~PaymentCode() final = default;

private:
    friend opentxs::Factory;

    const api::internal::Core& api_;
    const std::uint8_t version_;
    const bool hasBitmessage_;
    const OTData pubkey_;
    const OTSecret chain_code_;
    const std::uint8_t bitmessage_version_;
    const std::uint8_t bitmessage_stream_;
    const OTNymID id_;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    std::shared_ptr<crypto::key::Secp256k1> key_;
#else
    OTAsymmetricKey key_;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

    static auto calculate_id(
        const api::internal::Core& api,
        const ReadView pubkey,
        const ReadView chaincode) noexcept -> OTNymID;

    auto clone() const noexcept -> PaymentCode* final
    {
        return new PaymentCode(*this);
    }

    PaymentCode() = delete;
    PaymentCode(const PaymentCode&);
    PaymentCode(PaymentCode&&) = delete;
    auto operator=(const PaymentCode&) -> PaymentCode&;
    auto operator=(PaymentCode&&) -> PaymentCode&;
};
}  // namespace opentxs::implementation

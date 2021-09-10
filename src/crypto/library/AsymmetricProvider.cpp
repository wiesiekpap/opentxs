// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "crypto/library/AsymmetricProvider.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <cstddef>
#include <cstring>
#include <vector>

#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Primitives.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "util/Sodium.hpp"

#define OT_METHOD "opentxs::crypto::AsymmetricProvider::"

namespace opentxs::crypto
{
auto AsymmetricProvider::CurveToKeyType(const EcdsaCurve& curve)
    -> crypto::key::asymmetric::Algorithm
{
    crypto::key::asymmetric::Algorithm output =
        crypto::key::asymmetric::Algorithm::Error;

    switch (curve) {
        case (EcdsaCurve::secp256k1): {
            output = key::asymmetric::Algorithm::Secp256k1;

            break;
        }
        case (EcdsaCurve::ed25519): {
            output = key::asymmetric::Algorithm::ED25519;

            break;
        }
        default: {
        }
    }

    return output;
}

auto AsymmetricProvider::KeyTypeToCurve(
    const crypto::key::asymmetric::Algorithm& type) -> EcdsaCurve
{
    EcdsaCurve output = EcdsaCurve::invalid;

    switch (type) {
        case (key::asymmetric::Algorithm::Secp256k1): {
            output = EcdsaCurve::secp256k1;

            break;
        }
        case (key::asymmetric::Algorithm::ED25519): {
            output = EcdsaCurve::ed25519;

            break;
        }
        default: {
        }
    }

    return output;
}
}  // namespace opentxs::crypto

namespace opentxs::crypto::implementation
{
AsymmetricProvider::AsymmetricProvider() noexcept
{
    if (0 > ::sodium_init()) { OT_FAIL; }
}

auto AsymmetricProvider::SeedToCurveKey(
    const ReadView seed,
    const AllocateOutput privateKey,
    const AllocateOutput publicKey) const noexcept -> bool
{
    auto edPublic = Data::Factory();
    auto edPrivate = Context().Factory().Secret(0);

    if (false == sodium::ExpandSeed(
                     seed,
                     edPrivate->WriteInto(Secret::Mode::Mem),
                     edPublic->WriteInto())) {
        LogOutput(OT_METHOD)(__func__)(": Failed to expand seed.").Flush();

        return false;
    }

    if (false == bool(privateKey) || false == bool(publicKey)) {
        LogOutput(OT_METHOD)(__func__)(": Invalid output allocator").Flush();

        return false;
    }

    return sodium::ToCurveKeypair(
        edPrivate->Bytes(), edPublic->Bytes(), privateKey, publicKey);
}

auto AsymmetricProvider::SignContract(
    const api::Core& api,
    const String& contract,
    const ReadView theKey,
    const crypto::HashType hashType,
    Signature& output) const -> bool
{
    const auto plaintext = [&] {
        const auto size = std::size_t{contract.GetLength()};
        auto out = space(size + 1u);
        std::memcpy(out.data(), contract.Get(), size);
        out.back() = std::byte{0x0};

        return out;
    }();
    auto signature = api.Factory().Data();
    bool success =
        Sign(reader(plaintext), theKey, hashType, signature->WriteInto());
    output.SetData(signature, true);  // true means, "yes, with newlines in the
                                      // b64-encoded output, please."

    if (false == success) {
        LogOutput(OT_METHOD)(__func__)(": Failed to sign contract").Flush();
    }

    return success;
}

auto AsymmetricProvider::VerifyContractSignature(
    const api::Core& api,
    const String& contract,
    const ReadView key,
    const Signature& theSignature,
    const crypto::HashType hashType) const -> bool
{
    const auto plaintext = [&] {
        const auto size = std::size_t{contract.GetLength()};
        auto out = space(size + 1);
        std::memcpy(out.data(), contract.Get(), size);
        out.back() = std::byte{0x0};

        return out;
    }();
    const auto signature = [&] {
        auto out = api.Factory().Data();
        theSignature.GetData(out);

        return out;
    }();

    return Verify(reader(plaintext), key, signature->Bytes(), hashType);
}
}  // namespace opentxs::crypto::implementation

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_SYMMETRIC_HPP
#define OPENTXS_API_CRYPTO_SYMMETRIC_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/crypto/key/symmetric/Source.hpp"

namespace opentxs
{
namespace proto
{
class SymmetricKey;
}  // namespace proto

class Secret;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace crypto
{
class OPENTXS_EXPORT Symmetric
{
public:
    virtual std::size_t IvSize(
        const opentxs::crypto::key::symmetric::Algorithm mode) const = 0;
    virtual OTSymmetricKey Key(
        const PasswordPrompt& password,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305)
        const = 0;
    OPENTXS_NO_EXPORT virtual OTSymmetricKey Key(
        const proto::SymmetricKey& serialized,
        const opentxs::crypto::key::symmetric::Algorithm mode) const = 0;
    virtual OTSymmetricKey Key(
        const ReadView& serializedCiphertext,
        const opentxs::crypto::key::symmetric::Algorithm mode) const = 0;
    virtual OTSymmetricKey Key(
        const Secret& seed,
        const std::uint64_t operations = 0,
        const std::uint64_t difficulty = 0,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305,
        const opentxs::crypto::key::symmetric::Source type =
            opentxs::crypto::key::symmetric::Source::Argon2) const = 0;

    OPENTXS_NO_EXPORT virtual ~Symmetric() = default;

protected:
    Symmetric() = default;

private:
    Symmetric(const Symmetric&) = delete;
    Symmetric(Symmetric&&) = delete;
    Symmetric& operator=(const Symmetric&) = delete;
    Symmetric& operator=(Symmetric&&) = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif

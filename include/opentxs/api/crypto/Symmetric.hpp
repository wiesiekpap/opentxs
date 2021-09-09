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
    virtual auto IvSize(const opentxs::crypto::key::symmetric::Algorithm mode)
        const -> std::size_t = 0;
    virtual auto Key(
        const PasswordPrompt& password,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305) const
        -> OTSymmetricKey = 0;
    OPENTXS_NO_EXPORT virtual auto Key(
        const proto::SymmetricKey& serialized,
        const opentxs::crypto::key::symmetric::Algorithm mode) const
        -> OTSymmetricKey = 0;
    virtual auto Key(
        const ReadView& serializedCiphertext,
        const opentxs::crypto::key::symmetric::Algorithm mode) const
        -> OTSymmetricKey = 0;
    virtual auto Key(
        const Secret& seed,
        const std::uint64_t operations = 0,
        const std::uint64_t difficulty = 0,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305,
        const opentxs::crypto::key::symmetric::Source type =
            opentxs::crypto::key::symmetric::Source::Argon2i) const
        -> OTSymmetricKey = 0;
    virtual auto Key(
        const Secret& seed,
        const ReadView salt,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::uint64_t parallel,
        const std::size_t bytes,
        const opentxs::crypto::key::symmetric::Source type) const
        -> OTSymmetricKey = 0;

    OPENTXS_NO_EXPORT virtual ~Symmetric() = default;

protected:
    Symmetric() = default;

private:
    Symmetric(const Symmetric&) = delete;
    Symmetric(Symmetric&&) = delete;
    auto operator=(const Symmetric&) -> Symmetric& = delete;
    auto operator=(Symmetric&&) -> Symmetric& = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_HD_HPP
#define OPENTXS_CRYPTO_KEY_HD_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Hash;
}  // namespace crypto
}  // namespace api

namespace crypto
{
namespace key
{
class HD;
}  // namespace key
}  // namespace crypto

using OTHDKey = Pimpl<crypto::key::HD>;
}  // namespace opentxs

namespace opentxs
{
namespace crypto
{
namespace key
{
class HD : virtual public EllipticCurve
{
public:
    OPENTXS_EXPORT static Bip32Fingerprint CalculateFingerprint(
        const api::crypto::Hash& hash,
        const ReadView pubkey) noexcept;

    OPENTXS_EXPORT virtual ReadView Chaincode(
        const PasswordPrompt& reason) const noexcept = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<HD> ChildKey(
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept = 0;
    OPENTXS_EXPORT virtual int Depth() const noexcept = 0;
    OPENTXS_EXPORT virtual Bip32Fingerprint Fingerprint() const noexcept = 0;
    OPENTXS_EXPORT virtual Bip32Fingerprint Parent() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Xprv(
        const PasswordPrompt& reason) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Xpub(
        const PasswordPrompt& reason) const noexcept = 0;

    OPENTXS_EXPORT ~HD() override = default;

protected:
    HD() = default;

private:
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    HD& operator=(const HD&) = delete;
    HD& operator=(HD&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif

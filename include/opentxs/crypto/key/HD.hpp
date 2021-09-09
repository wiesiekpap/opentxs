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
class OPENTXS_EXPORT HD : virtual public EllipticCurve
{
public:
    static auto CalculateFingerprint(
        const api::crypto::Hash& hash,
        const ReadView pubkey) noexcept -> Bip32Fingerprint;

    virtual auto Chaincode(const PasswordPrompt& reason) const noexcept
        -> ReadView = 0;
    virtual auto ChildKey(const Bip32Index index, const PasswordPrompt& reason)
        const noexcept -> std::unique_ptr<HD> = 0;
    virtual auto Depth() const noexcept -> int = 0;
    virtual auto Fingerprint() const noexcept -> Bip32Fingerprint = 0;
    virtual auto Parent() const noexcept -> Bip32Fingerprint = 0;
    virtual auto Xprv(const PasswordPrompt& reason) const noexcept
        -> std::string = 0;
    virtual auto Xpub(const PasswordPrompt& reason) const noexcept
        -> std::string = 0;

    ~HD() override = default;

protected:
    HD() = default;

private:
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif

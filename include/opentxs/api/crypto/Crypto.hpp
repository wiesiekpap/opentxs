// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace api
{
namespace crypto
{
class Config;
class Encode;
class Hash;
class Util;
}  // namespace crypto

namespace internal
{
class Crypto;
}  // namespace internal
}  // namespace api

namespace crypto
{
class Bip32;
class Bip39;
}  // namespace crypto
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class OPENTXS_EXPORT Crypto
{
public:
    virtual auto BIP32() const noexcept -> const opentxs::crypto::Bip32& = 0;
    virtual auto BIP39() const noexcept -> const opentxs::crypto::Bip39& = 0;
    virtual auto Config() const noexcept -> const crypto::Config& = 0;
    virtual auto Encode() const noexcept -> const crypto::Encode& = 0;
    virtual auto Hash() const noexcept -> const crypto::Hash& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Crypto& = 0;
    virtual auto Util() const noexcept -> const crypto::Util& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Crypto& = 0;

    OPENTXS_NO_EXPORT virtual ~Crypto() = default;

protected:
    Crypto() = default;

private:
    Crypto(const Crypto&) = delete;
    Crypto(Crypto&&) = delete;
    auto operator=(const Crypto&) -> Crypto& = delete;
    auto operator=(Crypto&&) -> Crypto& = delete;
};
}  // namespace api
}  // namespace opentxs

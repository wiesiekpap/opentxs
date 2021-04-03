// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_CONFIG_HPP
#define OPENTXS_API_CRYPTO_CONFIG_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs
{
namespace api
{
namespace crypto
{
class Config
{
public:
    OPENTXS_EXPORT virtual std::uint32_t IterationCount() const = 0;
    OPENTXS_EXPORT virtual std::uint32_t SymmetricSaltSize() const = 0;
    OPENTXS_EXPORT virtual std::uint32_t SymmetricKeySize() const = 0;
    OPENTXS_EXPORT virtual std::uint32_t SymmetricKeySizeMax() const = 0;
    OPENTXS_EXPORT virtual std::uint32_t SymmetricIvSize() const = 0;
    OPENTXS_EXPORT virtual std::uint32_t SymmetricBufferSize() const = 0;
    OPENTXS_EXPORT virtual std::uint32_t PublicKeysize() const = 0;
    OPENTXS_EXPORT virtual std::uint32_t PublicKeysizeMax() const = 0;

    virtual ~Config() = default;

protected:
    Config() = default;

private:
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(const Config&) = delete;
    Config& operator=(Config&&) = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif

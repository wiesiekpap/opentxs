// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs
{
namespace api
{
namespace crypto
{
namespace internal
{
class Config;
}  // namespace internal
}  // namespace crypto
}  // namespace api
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace crypto
{
class OPENTXS_EXPORT Config
{
public:
    OPENTXS_NO_EXPORT virtual auto InternalConfig() const noexcept
        -> const internal::Config& = 0;
    virtual auto IterationCount() const -> std::uint32_t = 0;
    virtual auto SymmetricSaltSize() const -> std::uint32_t = 0;
    virtual auto SymmetricKeySize() const -> std::uint32_t = 0;
    virtual auto SymmetricKeySizeMax() const -> std::uint32_t = 0;
    virtual auto SymmetricIvSize() const -> std::uint32_t = 0;
    virtual auto SymmetricBufferSize() const -> std::uint32_t = 0;
    virtual auto PublicKeysize() const -> std::uint32_t = 0;
    virtual auto PublicKeysizeMax() const -> std::uint32_t = 0;

    OPENTXS_NO_EXPORT virtual auto InternalConfig() noexcept
        -> internal::Config& = 0;

    OPENTXS_NO_EXPORT virtual ~Config() = default;

protected:
    Config() = default;

private:
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    auto operator=(const Config&) -> Config& = delete;
    auto operator=(Config&&) -> Config& = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/Bip32.hpp"

#pragma once

#include <memory>

namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace crypto
{
class Bip32;
}  // namespace crypto
}  // namespace opentxs

namespace opentxs::factory
{
auto Bip32(const api::Crypto& crypto) noexcept -> crypto::Bip32;
}  // namespace opentxs::factory

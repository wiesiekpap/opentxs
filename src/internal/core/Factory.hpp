// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class PaymentCode;
}  // namespace proto

class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs

namespace opentxs::factory
{
auto PaymentCode(
    const api::Session& api,
    const UnallocatedCString& base58) noexcept -> opentxs::PaymentCode;
auto PaymentCode(
    const api::Session& api,
    const proto::PaymentCode& serialized) noexcept -> opentxs::PaymentCode;
auto PaymentCode(
    const api::Session& api,
    const UnallocatedCString& seed,
    const Bip32Index nym,
    const std::uint8_t version,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream,
    const opentxs::PasswordPrompt& reason) noexcept -> opentxs::PaymentCode;
}  // namespace opentxs::factory

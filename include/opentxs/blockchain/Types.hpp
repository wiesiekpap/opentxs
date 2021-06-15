// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_TYPES_HPP
#define OPENTXS_BLOCKCHAIN_TYPES_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>
#include <tuple>
#include <utility>

namespace opentxs
{
namespace blockchain
{
namespace block
{
using Version = std::int32_t;
}  // namespace block

using TypeEnum = std::uint32_t;

namespace filter
{
enum class Type : TypeEnum;
}  // namespace filter

enum class Type : TypeEnum;
enum class BloomUpdateFlag : std::uint8_t;
enum class SendResult : TypeEnum;

using Amount = std::uint64_t;
using ChainHeight = std::int64_t;
using HDIndex = std::uint32_t;
using ConfirmedBalance = Amount;
using UnconfirmedBalance = Amount;
using Balance = std::pair<ConfirmedBalance, UnconfirmedBalance>;
}  // namespace blockchain

OPENTXS_EXPORT auto print(blockchain::SendResult) noexcept -> std::string;
OPENTXS_EXPORT auto print(blockchain::Type) noexcept -> std::string;
}  // namespace opentxs
#endif

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/bitcoin/block/Opcodes.hpp"

#pragma once

#include "opentxs/blockchain/block/Types.hpp"

#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::blockchain::bitcoin::block::internal
{
auto DecodeBip34(const ReadView coinbase) noexcept -> blockchain::block::Height;
auto EncodeBip34(blockchain::block::Height height) noexcept -> Space;
auto Opcode(const OP opcode) noexcept(false) -> ScriptElement;
auto PushData(const ReadView data) noexcept(false) -> ScriptElement;
}  // namespace opentxs::blockchain::bitcoin::block::internal

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::blockchain::database::wallet::db
{
struct Pattern {
    const Space data_;

    auto Data() const noexcept -> ReadView;
    auto Index() const noexcept -> Bip32Index;

    Pattern(const Bip32Index index, const ReadView data) noexcept;
    Pattern(const ReadView bytes) noexcept(false);

    ~Pattern() = default;

private:
    static constexpr auto fixed_ = sizeof(Bip32Index);

    Pattern() = delete;
    Pattern(const Pattern&) = delete;
    Pattern(Pattern&&) = delete;
    auto operator=(const Pattern&) -> Pattern& = delete;
    auto operator=(Pattern&&) -> Pattern& = delete;
};
}  // namespace opentxs::blockchain::database::wallet::db

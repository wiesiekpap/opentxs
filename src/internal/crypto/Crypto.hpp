// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs
{
namespace api
{
class Primitives;
}  // namespace api
}  // namespace opentxs

namespace opentxs::crypto::internal
{
struct Bip32 {
    virtual auto Init(const api::Primitives& factory) noexcept -> void = 0;

    virtual ~Bip32() = default;
};
}  // namespace opentxs::crypto::internal

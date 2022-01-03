// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Seed.hpp"

namespace opentxs
{
namespace proto
{
class HDPath;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::api::crypto::internal
{
class Seed : virtual public crypto::Seed
{
public:
    using api::crypto::Seed::AccountChildKey;
    virtual auto AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto AccountKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    auto Internal() const noexcept -> const Seed& final { return *this; }

    auto Internal() noexcept -> Seed& final { return *this; }

    ~Seed() override = default;
};
}  // namespace opentxs::api::crypto::internal

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_map.hpp>
#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

#include "Proto.hpp"
#include "blockchain/crypto/Subaccount.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/BlockchainDeterministicAccountData.pb.h"
#include "serialization/protobuf/HDPath.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Account;
}  // namespace crypto
}  // namespace blockchain

class Identifier;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class Notification final : virtual public Subaccount
{
public:
    auto AllowedSubchains() const noexcept -> UnallocatedSet<Subchain> final
    {
        return {};
    }
    auto BalanceElement(const Subchain, const Bip32Index) const noexcept(false)
        -> const crypto::Element& final
    {
        throw std::out_of_range{
            "no balance elements present in notification subaccounts"};
    }

    Notification(
        const api::Session& api,
        const crypto::Account& parent,
        OTIdentifier&& id) noexcept;

    ~Notification() final = default;

private:
    auto account_already_exists(const rLock&) const noexcept -> bool final
    {
        return false;
    }
    auto check_activity(
        const rLock&,
        const UnallocatedVector<Activity>&,
        UnallocatedSet<OTIdentifier>&,
        const PasswordPrompt&) const noexcept -> bool final
    {
        return false;
    }
    auto save(const rLock&) const noexcept -> bool final { return false; }

    auto mutable_element(
        const rLock&,
        const Subchain,
        const Bip32Index) noexcept(false) -> Element& final
    {
        throw std::out_of_range{
            "no balance elements present in notification subaccounts"};
    }

    Notification(
        const api::Session& api,
        const crypto::Account& parent,
        OTIdentifier&& id,
        OTIdentifier out) noexcept;

    Notification(const Notification&) = delete;
    Notification(Notification&&) = delete;
    auto operator=(const Notification&) -> Notification& = delete;
    auto operator=(Notification&&) -> Notification& = delete;
};
}  // namespace opentxs::blockchain::crypto::implementation

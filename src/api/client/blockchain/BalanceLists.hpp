// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "api/client/Blockchain.hpp"
#include "blockchain/crypto/AccountIndex.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace blockchain
{
namespace crypto
{
namespace internal
{
struct Wallet;
}  // namespace internal
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
struct BalanceLists {
    using AccountData = Blockchain::AccountData;
    using Chain = Blockchain::Chain;

    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> std::set<OTIdentifier>;
    auto AccountList(const Chain chain) const noexcept
        -> std::set<OTIdentifier>;
    auto AccountList() const noexcept -> std::set<OTIdentifier>;
    auto Get(const Chain chain) noexcept
        -> opentxs::blockchain::crypto::internal::Wallet&;
    auto LookupAccount(const Identifier& id) const noexcept -> AccountData;

    BalanceLists(
        const api::internal::Core& api,
        api::client::internal::Blockchain& parent) noexcept;

private:
    const api::internal::Core& api_;
    api::client::internal::Blockchain& parent_;
    internal::BalanceTreeIndex index_;
    mutable std::mutex lock_;
    mutable bool populated_;
    mutable std::map<
        Chain,
        std::unique_ptr<opentxs::blockchain::crypto::internal::Wallet>>
        lists_;

    auto get(const Lock& lock, const Chain chain) const noexcept
        -> opentxs::blockchain::crypto::internal::Wallet&;
    auto populate() const noexcept -> void;
    auto populate(const Lock& lock) const noexcept -> void;
};
}  // namespace opentxs::api::client::implementation

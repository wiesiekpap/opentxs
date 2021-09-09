// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "blockchain/crypto/AccountIndex.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
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
class Blockchain;
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Wallet;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs

namespace opentxs::api::client::blockchain
{
class Wallets
{
public:
    using AccountData = client::Blockchain::AccountData;

    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> std::set<OTIdentifier>;
    auto AccountList(const opentxs::blockchain::Type chain) const noexcept
        -> std::set<OTIdentifier>;
    auto AccountList() const noexcept -> std::set<OTIdentifier>;
    auto Get(const opentxs::blockchain::Type chain) noexcept
        -> opentxs::blockchain::crypto::Wallet&;
    auto LookupAccount(const Identifier& id) const noexcept -> AccountData;

    Wallets(const api::Core& api, api::client::Blockchain& parent) noexcept;

private:
    const api::Core& api_;
    api::client::Blockchain& parent_;
    opentxs::blockchain::crypto::AccountIndex index_;
    mutable std::mutex lock_;
    mutable bool populated_;
    mutable std::map<
        opentxs::blockchain::Type,
        std::unique_ptr<opentxs::blockchain::crypto::Wallet>>
        lists_;

    auto get(const Lock& lock, const opentxs::blockchain::Type chain)
        const noexcept -> opentxs::blockchain::crypto::Wallet&;
    auto populate() const noexcept -> void;
    auto populate(const Lock& lock) const noexcept -> void;
};
}  // namespace opentxs::api::client::blockchain

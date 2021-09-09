// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"

#pragma once

#include <set>

#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Identifier.hpp"

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
class Account;
class AccountIndex;
class HD;
class PaymentCode;
class Wallet;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class Bip47Channel;
class HDAccount;
class HDPath;
}  // namespace proto

class Data;
class Identifier;
class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs

namespace opentxs::factory
{
auto BlockchainAccountKeys(
    const api::Core& api,
    const blockchain::crypto::Wallet& parent,
    const blockchain::crypto::AccountIndex& index,
    const identifier::Nym& id,
    const std::set<OTIdentifier>& hdAccounts,
    const std::set<OTIdentifier>& importedAccounts,
    const std::set<OTIdentifier>& paymentCodeAccounts) noexcept
    -> std::unique_ptr<blockchain::crypto::Account>;
auto BlockchainHDSubaccount(
    const api::Core& api,
    const blockchain::crypto::Account& parent,
    const proto::HDPath& path,
    const blockchain::crypto::HDProtocol standard,
    const PasswordPrompt& reason,
    Identifier& id) noexcept -> std::unique_ptr<blockchain::crypto::HD>;
auto BlockchainHDSubaccount(
    const api::Core& api,
    const blockchain::crypto::Account& parent,
    const proto::HDAccount& serialized,
    Identifier& id) noexcept -> std::unique_ptr<blockchain::crypto::HD>;
auto BlockchainPCSubaccount(
    const api::Core& api,
    const blockchain::crypto::Account& parent,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const Data& txid,
    const PasswordPrompt& reason,
    Identifier& id) noexcept
    -> std::unique_ptr<blockchain::crypto::PaymentCode>;
auto BlockchainPCSubaccount(
    const api::Core& api,
    const blockchain::crypto::Account& parent,
    const proto::Bip47Channel& serialized,
    Identifier& id) noexcept
    -> std::unique_ptr<blockchain::crypto::PaymentCode>;
auto BlockchainWalletKeys(
    const api::Core& api,
    const api::client::Blockchain& parent,
    const blockchain::crypto::AccountIndex& index,
    const blockchain::Type chain) noexcept
    -> std::unique_ptr<blockchain::crypto::Wallet>;
}  // namespace opentxs::factory

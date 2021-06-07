// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <set>

#include "opentxs/core/Identifier.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
class BalanceTreeIndex;
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
struct Account;
struct HD;
struct PaymentCode;
}  // namespace internal
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
auto BlockchainBalanceList(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& parent,
    const api::client::internal::BalanceTreeIndex& index,
    const blockchain::Type chain) noexcept
    -> std::unique_ptr<blockchain::crypto::internal::Wallet>;
auto BlockchainBalanceTree(
    const api::internal::Core& api,
    const blockchain::crypto::internal::Wallet& parent,
    const api::client::internal::BalanceTreeIndex& index,
    const identifier::Nym& id,
    const std::set<OTIdentifier>& hdAccounts,
    const std::set<OTIdentifier>& importedAccounts,
    const std::set<OTIdentifier>& paymentCodeAccounts) noexcept
    -> std::unique_ptr<blockchain::crypto::internal::Account>;
auto BlockchainHDBalanceNode(
    const api::internal::Core& api,
    const blockchain::crypto::internal::Account& parent,
    const proto::HDPath& path,
    const PasswordPrompt& reason,
    Identifier& id) noexcept
    -> std::unique_ptr<blockchain::crypto::internal::HD>;
auto BlockchainHDBalanceNode(
    const api::internal::Core& api,
    const blockchain::crypto::internal::Account& parent,
    const proto::HDAccount& serialized,
    Identifier& id) noexcept
    -> std::unique_ptr<blockchain::crypto::internal::HD>;
auto BlockchainPCBalanceNode(
    const api::internal::Core& api,
    const blockchain::crypto::internal::Account& parent,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const Data& txid,
    const PasswordPrompt& reason,
    Identifier& id) noexcept
    -> std::unique_ptr<blockchain::crypto::internal::PaymentCode>;
auto BlockchainPCBalanceNode(
    const api::internal::Core& api,
    const blockchain::crypto::internal::Account& parent,
    const proto::Bip47Channel& serialized,
    Identifier& id) noexcept
    -> std::unique_ptr<blockchain::crypto::internal::PaymentCode>;
}  // namespace opentxs::factory

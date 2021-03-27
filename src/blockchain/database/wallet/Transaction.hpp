// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "api/client/blockchain/database/Database.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace database
{
namespace implementation
{
class Database;
}  // namespace implementation
}  // namespace database
}  // namespace blockchain

namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::blockchain::database::wallet
{
class Transaction
{
public:
    using Common = api::client::blockchain::database::implementation::Database;

    auto TransactionLoadBitcoin(const ReadView txid) const noexcept
        -> std::unique_ptr<block::bitcoin::Transaction>;

    auto Add(
        const blockchain::Type chain,
        const block::Position& block,
        const block::bitcoin::Transaction& transaction,
        const PasswordPrompt& reason) noexcept -> bool;
    auto Rollback(const block::Height block, const block::Txid& txid) noexcept
        -> bool;

    Transaction(
        const api::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const Common& common) noexcept;

    ~Transaction();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::wallet

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/database/wallet/Transaction.hpp"  // IWYU pragma: associated

#include <map>
#include <mutex>
#include <optional>
#include <set>

#include "blockchain/database/common/Database.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::blockchain::database::wallet::Transaction::"

namespace opentxs::blockchain::database::wallet
{
struct Transaction::Imp {
    auto TransactionLoadBitcoin(const ReadView txid) const noexcept
        -> std::unique_ptr<block::bitcoin::Transaction>
    {
        const auto serialized = common_.LoadTransaction(txid);

        if (false == serialized.has_value()) { return {}; }

        return factory::BitcoinTransaction(
            api_, blockchain_, serialized.value());
    }

    auto Add(
        const blockchain::Type chain,
        const block::Position& block,
        const block::bitcoin::Transaction& transaction,
        const PasswordPrompt& reason) noexcept -> bool
    {
        auto lock = Lock{lock_};
        const auto& [height, blockHash] = block;

        {
            auto& index = tx_to_block_[transaction.ID()];
            index.emplace(blockHash);
        }

        {
            auto& index = block_to_tx_[blockHash];
            index.emplace(transaction.ID());
        }

        {
            auto& index = tx_history_[height];
            index.emplace(transaction.ID());
        }

        return blockchain_.ProcessTransaction(chain, transaction, reason);
    }
    auto Rollback(const block::Height block, const block::Txid& txid) noexcept
        -> bool
    {
        try {
            auto& history = tx_history_.at(block);
            history.erase(txid);

            return true;
        } catch (...) {
            LogOutput(OT_METHOD)(__func__)(
                ": No transaction history at block ")(block)
                .Flush();

            return false;
        }
    }

    Imp(const api::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const database::common::Database& common) noexcept
        : api_(api)
        , blockchain_(blockchain)
        , common_(common)
        , lock_()
        , tx_to_block_()
        , block_to_tx_()
        , tx_history_()
    {
    }

private:
    using TransactionBlockMap = std::map<block::pTxid, std::set<block::pHash>>;
    using BlockTransactionMap = std::map<block::pHash, std::set<block::pTxid>>;
    using TransactionHistory = std::map<block::Height, std::set<block::pTxid>>;

    const api::Core& api_;
    const api::client::internal::Blockchain& blockchain_;
    const database::common::Database& common_;
    mutable std::mutex lock_;
    mutable TransactionBlockMap tx_to_block_;
    mutable BlockTransactionMap block_to_tx_;
    mutable TransactionHistory tx_history_;
};

Transaction::Transaction(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const database::common::Database& common) noexcept
    : imp_(std::make_unique<Imp>(api, blockchain, common))
{
    OT_ASSERT(imp_);
}

auto Transaction::Add(
    const blockchain::Type chain,
    const block::Position& block,
    const block::bitcoin::Transaction& transaction,
    const PasswordPrompt& reason) noexcept -> bool
{
    return imp_->Add(chain, block, transaction, reason);
}

auto Transaction::Rollback(
    const block::Height block,
    const block::Txid& txid) noexcept -> bool
{
    return imp_->Rollback(block, txid);
}

auto Transaction::TransactionLoadBitcoin(const ReadView txid) const noexcept
    -> std::unique_ptr<block::bitcoin::Transaction>
{
    return imp_->TransactionLoadBitcoin(txid);
}

Transaction::~Transaction() = default;
}  // namespace opentxs::blockchain::database::wallet

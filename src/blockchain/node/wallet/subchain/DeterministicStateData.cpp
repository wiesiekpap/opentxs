// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <chrono>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Outputs.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "util/Container.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
DeterministicStateData::DeterministicStateData(
    const api::Session& api,
    const node::internal::Manager& node,
    database::Wallet& db,
    const node::internal::Mempool& mempool,
    const crypto::Deterministic& subaccount,
    const cfilter::Type filter,
    const crypto::Subchain subchain,
    const network::zeromq::BatchID batch,
    const std::string_view parent,
    allocator_type alloc) noexcept
    : SubchainStateData(
          api,
          node,
          db,
          mempool,
          subaccount,
          filter,
          subchain,
          batch,
          parent,
          std::move(alloc))
    , deterministic_(subaccount)
    , cache_(Clock::now(), get_allocator())
{
}

auto DeterministicStateData::CheckCache(
    const std::size_t outstanding,
    FinishedCallback cb) const noexcept -> void
{
    cache_.modify([=](auto& data) {
        auto& [time, blockMap] = data;
        using namespace std::literals;
        static constexpr auto maxTime = 10s;
        const auto interval = Clock::now() - time;
        const auto flush = (0u == outstanding) || (interval > maxTime);

        if (flush) {
            flush_cache(blockMap, cb);
            time = Clock::now();
        }
    });
}

auto DeterministicStateData::flush_cache(
    database::Wallet::BatchedMatches& matches,
    FinishedCallback cb) const noexcept -> void
{
    const auto start = Clock::now();
    const auto& log = log_;

    if (0u < matches.size()) {
        auto txoCreated = TXOs{get_allocator()};
        auto txoConsumed = TXOs{get_allocator()};
        auto positions = Vector<block::Position>{get_allocator()};
        positions.reserve(matches.size());
        std::transform(
            matches.begin(),
            matches.end(),
            std::back_inserter(positions),
            [](const auto& data) { return data.first; });
        auto updated = db_.AddConfirmedTransactions(
            id_, db_key_, std::move(matches), txoCreated, txoConsumed);

        OT_ASSERT(updated);  // TODO handle database errors

        element_cache_.lock()->Add(
            std::move(txoCreated), std::move(txoConsumed));

        if (cb) { cb(positions); }
    } else {
        log(OT_PRETTY_CLASS())(name())(" no cached transactions").Flush();
    }

    log(OT_PRETTY_CLASS())(name())(" finished flushing cache in ")(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            Clock::now() - start))
        .Flush();
}

auto DeterministicStateData::get_index(
    const boost::shared_ptr<const SubchainStateData>& me) const noexcept
    -> Index
{
    return Index::DeterministicFactory(me, *this);
}

auto DeterministicStateData::handle_confirmed_matches(
    const bitcoin::block::Block& block,
    const block::Position& position,
    const block::Matches& confirmed,
    const Log& log) const noexcept -> void
{
    const auto start = Clock::now();
    const auto& [utxo, general] = confirmed;
    auto transactions = database::Wallet::BlockMatches{get_allocator()};

    for (const auto& match : general) {
        const auto& [txid, elementID] = match;
        const auto& pTransaction = block.at(txid->Bytes());

        OT_ASSERT(pTransaction);

        auto& arg = transactions[txid];
        auto postcondition = ScopeGuard{[&] {
            if (nullptr == arg.second) { transactions.erase(match.first); }
        }};
        const auto& transaction = *pTransaction;
        process(match, transaction, arg);
    }

    const auto processMatches = Clock::now();

    for (const auto& [tx, outpoint, element] : utxo) {
        auto& pTx = transactions[tx].second;

        if (!pTx) { pTx = block.at(tx->Bytes())->clone(); }
    }

    const auto buildTransactionMap = Clock::now();
    log(OT_PRETTY_CLASS())(name())(" adding ")(transactions.size())(
        " confirmed transaction(s) to cache")
        .Flush();
    cache_.modify([this, &log, position, matches = std::move(transactions)](
                      auto& data) {
        auto& [time, blockMap] = data;

        if (auto i = blockMap.find(position); blockMap.end() == i) {
            blockMap.emplace(std::move(position), std::move(matches));
        } else {
            auto& existing = i->second;

            for (auto& [txid, newMatchData] : matches) {
                if (auto e = existing.find(txid); existing.end() == e) {
                    log(OT_PRETTY_CLASS())(name())(" adding transaction ")
                        .asHex(txid)(" to cache")
                        .Flush();
                    existing.emplace(std::move(txid), std::move(newMatchData));
                } else {
                    log(OT_PRETTY_CLASS())(name())(" updating transaction ")
                        .asHex(txid)(" metadata")
                        .Flush();
                    auto& [eIndices, eTX] = e->second;
                    const auto& [nIndices, nTX] = newMatchData;
                    eIndices.insert(
                        eIndices.end(), nIndices.begin(), nIndices.end());
                    dedup(eIndices);

                    OT_ASSERT(nTX);
                    OT_ASSERT(eTX);

                    eTX->Internal().MergeMetadata(chain_, nTX->Internal(), log);
                }
            }
        }
    });
    const auto updateCache = Clock::now();
    log(OT_PRETTY_CLASS())(name())(" time to process matches: ")(
        std::chrono::nanoseconds{processMatches - start})
        .Flush();
    log(OT_PRETTY_CLASS())(name())(" time to build transaction map: ")(
        std::chrono::nanoseconds{buildTransactionMap - processMatches})
        .Flush();
    log(OT_PRETTY_CLASS())(name())(" time to update cache: ")(
        std::chrono::nanoseconds{updateCache - buildTransactionMap})
        .Flush();
}

auto DeterministicStateData::handle_mempool_matches(
    const block::Matches& matches,
    std::unique_ptr<const bitcoin::block::Transaction> in) const noexcept
    -> void
{
    const auto& [utxo, general] = matches;

    if (0u == general.size()) { return; }

    auto data = database::Wallet::MatchedTransaction{};
    auto& [outputs, pTX] = data;

    for (const auto& match : general) { process(match, *in, data); }

    if (nullptr == pTX) { return; }

    const auto& tx = *pTX;
    auto txoCreated = TXOs{get_allocator()};
    auto updated =
        db_.AddMempoolTransaction(id_, subchain_, outputs, tx, txoCreated);

    OT_ASSERT(updated);  // TODO handle database errors

    element_cache_.lock()->Add(std::move(txoCreated), {});
    log_(OT_PRETTY_CLASS())(name())(
        " finished processing unconfirmed transaction ")(tx.ID().asHex())
        .Flush();
}

auto DeterministicStateData::process(
    const block::Match match,
    const bitcoin::block::Transaction& transaction,
    database::Wallet::MatchedTransaction& output) const noexcept -> void
{
    auto& [outputs, pTX] = output;
    const auto& [txid, elementID] = match;
    const auto& [index, subchainID] = elementID;
    const auto& [subchain, accountID] = subchainID;
    const auto& element = deterministic_.BalanceElement(subchain, index);
    set_key_data(const_cast<bitcoin::block::Transaction&>(transaction));
    auto i = Bip32Index{0};

    for (const auto& output : transaction.Outputs()) {
        if (crypto::Subchain::Outgoing == subchain_) { break; }

        auto post = ScopeGuard{[&] { ++i; }};
        const auto& script = output.Script();

        switch (script.Type()) {
            case bitcoin::block::Script::Pattern::PayToPubkey: {
                const auto pKey = element.Key();

                OT_ASSERT(pKey);
                OT_ASSERT(script.Pubkey().has_value());

                const auto& key = *pKey;

                if (key.PublicKey() == script.Pubkey().value()) {
                    log_(OT_PRETTY_CLASS())(name())(" element ")(
                        index)(": P2PK match found for ")(print(node_.Chain()))(
                        " transaction ")(txid->asHex())(" output ")(i)(" via ")(
                        api_.Factory().DataFromBytes(key.PublicKey())->asHex())
                        .Flush();
                    outputs.emplace_back(i);
                    const auto confirmed = api_.Crypto().Blockchain().Confirm(
                        element.KeyID(), txid);

                    OT_ASSERT(confirmed);

                    if (!pTX) { pTX = transaction.Internal().clone(); }
                }
            } break;
            case bitcoin::block::Script::Pattern::PayToPubkeyHash: {
                const auto hash = element.PubkeyHash();

                OT_ASSERT(script.PubkeyHash().has_value());

                if (hash->Bytes() == script.PubkeyHash().value()) {
                    log_(OT_PRETTY_CLASS())(name())(" element ")(
                        index)(": P2PKH match found for ")(
                        print(node_.Chain()))(" transaction ")(txid->asHex())(
                        " output ")(i)(" via ")(hash->asHex())
                        .Flush();
                    outputs.emplace_back(i);
                    const auto confirmed = api_.Crypto().Blockchain().Confirm(
                        element.KeyID(), txid);

                    OT_ASSERT(confirmed);

                    if (!pTX) { pTX = pTX = transaction.Internal().clone(); }
                }
            } break;
            case bitcoin::block::Script::Pattern::PayToWitnessPubkeyHash: {
                const auto hash = element.PubkeyHash();

                OT_ASSERT(script.PubkeyHash().has_value());

                if (hash->Bytes() == script.PubkeyHash().value()) {
                    log_(OT_PRETTY_CLASS())(name())(" element ")(
                        index)(": P2WPKH match found for ")(
                        print(node_.Chain()))(" transaction ")(txid->asHex())(
                        " output ")(i)(" via ")(hash->asHex())
                        .Flush();
                    outputs.emplace_back(i);
                    const auto confirmed = api_.Crypto().Blockchain().Confirm(
                        element.KeyID(), txid);

                    OT_ASSERT(confirmed);

                    if (!pTX) { pTX = pTX = transaction.Internal().clone(); }
                }
            } break;
            case bitcoin::block::Script::Pattern::PayToMultisig: {
                const auto m = script.M();
                const auto n = script.N();

                OT_ASSERT(m.has_value());
                OT_ASSERT(n.has_value());

                if (1u != m.value() || (3u != n.value())) {
                    // TODO handle non-payment code multisig eventually

                    continue;
                }

                const auto pKey = element.Key();

                OT_ASSERT(pKey);

                const auto& key = *pKey;

                if (key.PublicKey() == script.MultisigPubkey(0).value()) {
                    log_(OT_PRETTY_CLASS())(name())(" element ")(index)(": ")(
                        m.value())(" of ")(n.value())(" P2MS match found for ")(
                        print(node_.Chain()))(" transaction ")(txid->asHex())(
                        " output ")(i)(" via ")(
                        api_.Factory().DataFromBytes(key.PublicKey())->asHex())
                        .Flush();
                    outputs.emplace_back(i);
                    const auto confirmed = api_.Crypto().Blockchain().Confirm(
                        element.KeyID(), txid);

                    OT_ASSERT(confirmed);

                    if (!pTX) { pTX = pTX = transaction.Internal().clone(); }
                }
            } break;
            case bitcoin::block::Script::Pattern::PayToScriptHash:
            default: {
            }
        };
    }
}
}  // namespace opentxs::blockchain::node::wallet

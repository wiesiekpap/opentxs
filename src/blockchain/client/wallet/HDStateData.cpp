// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "blockchain/client/wallet/HDStateData.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <future>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <utility>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/blockchain/HD.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"  // IWYU pragma: keep
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::HDStateData::"

namespace opentxs::blockchain::client::implementation
{
HDStateData::HDStateData(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const internal::Network& network,
    const WalletDatabase& db,
    const api::client::blockchain::HD& node,
    const SimpleCallback& taskFinished,
    const filter::Type filter,
    const Subchain subchain) noexcept
    : api_(api)
    , blockchain_(blockchain)
    , network_(network)
    , db_(db)
    , node_(node)
    , task_finished_(taskFinished)
    , filter_type_(filter)
    , subchain_(subchain)
    , running_(false)
    , reorg_()
    , last_indexed_()
    , last_scanned_()
    , blocks_to_request_()
    , outstanding_blocks_()
    , process_block_queue_()
{
    OT_ASSERT(task_finished_);
}

auto HDStateData::ReorgQueue::Empty() const noexcept -> bool
{
    Lock lock(lock_);

    return 0 == parents_.size();
}

auto HDStateData::ReorgQueue::Queue(const block::Position& parent) noexcept
    -> bool
{
    Lock lock(lock_);
    parents_.push(parent);

    return true;
}

auto HDStateData::ReorgQueue::Next() noexcept -> block::Position
{
    OT_ASSERT(false == Empty());

    Lock lock(lock_);
    auto output{parents_.front()};
    parents_.pop();

    return output;
}

auto HDStateData::get_targets(
    const internal::WalletDatabase::Patterns& keys,
    const std::vector<internal::WalletDatabase::UTXO>& unspent) const noexcept
    -> client::GCS::Targets
{
    auto output = client::GCS::Targets{};
    output.reserve(keys.size() + unspent.size());
    std::transform(
        std::begin(keys),
        std::end(keys),
        std::back_inserter(output),
        [](const auto& in) { return reader(in.second); });

    switch (filter_type_) {
        case filter::Type::Basic_BIP158: {
            std::transform(
                std::begin(unspent),
                std::end(unspent),
                std::back_inserter(output),
                [](const auto& in) {
                    return ReadView{
                        in.second.script().data(), in.second.script().size()};
                });
        } break;
        case filter::Type::Basic_BCHVariant:
        case filter::Type::Extended_opentxs:
        default: {
            std::transform(
                std::begin(unspent),
                std::end(unspent),
                std::back_inserter(output),
                [](const auto& in) { return in.first.Bytes(); });
        }
    }

    return output;
}

auto HDStateData::index() noexcept -> void
{
    const auto first =
        last_indexed_.has_value() ? last_indexed_.value() + 1 : Bip32Index{0};
    const auto last = node_.LastGenerated(subchain_);
    auto elements = WalletDatabase::ElementMap{};

    for (auto i{first}; i <= last; ++i) {
        const auto& element = node_.BalanceElement(subchain_, i);
        index_element(filter_type_, element, i, elements);
    }

    db_.SubchainAddElements(node_.ID(), subchain_, filter_type_, elements);
}

auto HDStateData::index_element(
    const filter::Type type,
    const api::client::blockchain::BalanceNode::Element& input,
    const Bip32Index index,
    WalletDatabase::ElementMap& output) noexcept -> void
{
    const auto pubkeyHash = input.PubkeyHash();
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Indexing public key with hash ")(
        pubkeyHash->asHex())
        .Flush();
    auto& list = output[index];
    auto scripts = std::vector<std::unique_ptr<const block::bitcoin::Script>>{};
    scripts.reserve(4);  // WARNING keep this number up to date if new scripts
                         // are added
    const auto& p2pk = scripts.emplace_back(
        api_.Factory().BitcoinScriptP2PK(network_.Chain(), *input.Key()));
    const auto& p2pkh = scripts.emplace_back(
        api_.Factory().BitcoinScriptP2PKH(network_.Chain(), *input.Key()));
    const auto& p2sh_p2pk = scripts.emplace_back(
        api_.Factory().BitcoinScriptP2SH(network_.Chain(), *p2pk));
    const auto& p2sh_p2pkh = scripts.emplace_back(
        api_.Factory().BitcoinScriptP2SH(network_.Chain(), *p2pkh));

    OT_ASSERT(p2pk);
    OT_ASSERT(p2pkh);
    OT_ASSERT(p2sh_p2pk);
    OT_ASSERT(p2sh_p2pkh);

    switch (type) {
        case filter::Type::Extended_opentxs: {
            OT_ASSERT(p2pk->Pubkey().has_value());
            OT_ASSERT(p2pkh->PubkeyHash().has_value());
            OT_ASSERT(p2sh_p2pk->ScriptHash().has_value());
            OT_ASSERT(p2sh_p2pkh->ScriptHash().has_value());

            list.emplace_back(space(p2pk->Pubkey().value()));
            list.emplace_back(space(p2pkh->PubkeyHash().value()));
            list.emplace_back(space(p2sh_p2pk->ScriptHash().value()));
            list.emplace_back(space(p2sh_p2pkh->ScriptHash().value()));
        } break;
        case filter::Type::Basic_BIP158:
        case filter::Type::Basic_BCHVariant:
        default: {
            for (const auto& script : scripts) {
                script->Serialize(writer(list.emplace_back()));
            }
        }
    }
}

auto HDStateData::process() noexcept -> void
{
    const auto start = Clock::now();
    const auto& filters = network_.FilterOracleInternal();
    auto it = process_block_queue_.front();
    auto postcondition = ScopeGuard{[&] {
        outstanding_blocks_.erase(it);
        process_block_queue_.pop();
    }};
    const auto& blockHash = it->first.get();
    const auto pBlock = it->second.get();

    if (false == bool(pBlock)) {
        LogVerbose("opentxs::blockchain::client::internal::")(__FUNCTION__)(
            ": Invalid block ")(blockHash.asHex())
            .Flush();
        auto& vector = blocks_to_request_;
        vector.emplace(vector.begin(), blockHash);

        return;
    }

    const auto& block = *pBlock;
    auto tested = WalletDatabase::MatchingIndices{};
    auto patterns = client::GCS::Targets{};
    auto elements = db_.GetUntestedPatterns(
        node_.ID(), subchain_, filter_type_, blockHash.Bytes());
    std::for_each(
        std::begin(elements), std::end(elements), [&](const auto& pattern) {
            const auto& [id, data] = pattern;
            const auto& [index, subchain] = id;
            tested.emplace_back(index);
            patterns.emplace_back(reader(data));
        });

    const auto pFilter = filters.LoadFilter(filter_type_, blockHash);

    OT_ASSERT(pFilter);

    const auto& filter = *pFilter;
    auto potential = WalletDatabase::Patterns{};

    for (const auto& it : filter.Match(patterns)) {
        const auto pos = std::distance(patterns.cbegin(), it);
        auto& [id, element] = elements.at(pos);
        potential.emplace_back(std::move(id), std::move(element));
    }

    const auto confirmed =
        block.FindMatches(blockchain_, filter_type_, {}, potential);
    const auto& oracle = network_.HeaderOracleInternal();
    const auto pHeader = oracle.LoadHeader(blockHash);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;
    update_utxos(block, header.Position(), confirmed);
    const auto [balance, unconfirmed] = db_.GetBalance();
    LogVerbose("opentxs::blockchain::client::internal::")(__FUNCTION__)(
        ": block ")(blockHash.asHex())(" processed in ")(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start)
            .count())(" milliseconds. ")(confirmed.size())(" of ")(
        potential.size())(" potential matches confirmed. Wallet balance is: ")(
        unconfirmed)(" (")(balance)(" confirmed)")
        .Flush();
    db_.SubchainMatchBlock(
        node_.ID(), subchain_, filter_type_, tested, blockHash.Bytes());

    if (0 < confirmed.size()) {
        // Re-scan the last 1000 blocks
        const auto height = std::max(header.Height() - 1000, block::Height{0});
        last_scanned_ = block::Position{height, oracle.BestHash(height)};
    }
}

auto HDStateData::reorg() noexcept -> void
{
    while (false == reorg_.Empty()) {
        reorg_.Next();
        const auto tip =
            db_.SubchainLastProcessed(node_.ID(), subchain_, filter_type_);
        const auto reorg = network_.HeaderOracleInternal().CalculateReorg(tip);
        db_.ReorgTo(node_.ID(), subchain_, filter_type_, reorg);
    }

    const auto scannedTarget = network_.HeaderOracleInternal()
                                   .CommonParent(db_.SubchainLastScanned(
                                       node_.ID(), subchain_, filter_type_))
                                   .first;

    if (last_scanned_.has_value()) { last_scanned_ = scannedTarget; }

    blocks_to_request_.clear();
    outstanding_blocks_.clear();

    while (false == process_block_queue_.empty()) {
        process_block_queue_.pop();
    }
}

auto HDStateData::scan() noexcept -> void
{
    const auto start = Clock::now();
    const auto& headers = network_.HeaderOracleInternal();
    const auto& filters = network_.FilterOracleInternal();
    const auto best = headers.BestChain();
    const auto startHeight = std::min(
        best.first,
        last_scanned_.has_value() ? last_scanned_.value().first + 1 : 0);
    const auto first =
        last_scanned_.has_value()
            ? block::Position{startHeight, headers.BestHash(startHeight)}
            : block::Position{1, headers.BestHash(1)};
    const auto stopHeight = std::min(
        std::min(startHeight + 9999, best.first),
        filters.FilterTip(filter_type_).first);

    if (first.second->empty()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Resetting due to reorg").Flush();

        return;
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Scanning filters from ")(
            startHeight)(" to ")(stopHeight)
            .Flush();
    }

    const auto elements = db_.GetPatterns(node_.ID(), subchain_, filter_type_);
    const auto utxos = db_.GetUnspentOutputs();
    auto highestTested =
        last_scanned_.value_or(make_blank<block::Position>::value(api_));
    auto atLeastOnce{false};

    for (auto i{startHeight}; i <= stopHeight; ++i) {
        auto blockHash = headers.BestHash(i);
        const auto pFilter = filters.LoadFilterOrResetTip(
            filter_type_, block::Position{i, blockHash});

        if (false == bool(pFilter)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Filter at height ")(i)(
                " not found ")
                .Flush();

            break;
        }

        atLeastOnce = true;
        highestTested.first = i;
        highestTested.second = blockHash;
        const auto& filter = *pFilter;
        auto patterns = get_targets(elements, utxos);
        auto matches = filter.Match(patterns);
        const auto size{matches.size()};

        if (0 < matches.size()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": GCS for block ")(
                blockHash->asHex())(" at height ")(i)(
                " matches at least one of the ")(patterns.size())(
                " target elements for this subchain")
                .Flush();
            const auto retest = db_.GetUntestedPatterns(
                node_.ID(), subchain_, filter_type_, blockHash->Bytes());
            patterns = get_targets(retest, utxos);
            matches = filter.Match(patterns);
            LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(matches.size())(" of ")(
                size)(" matches are new")
                .Flush();

            if (0 < matches.size()) {
                blocks_to_request_.emplace_back(std::move(blockHash));
            }
        }
    }

    if (atLeastOnce) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Found ")(
            blocks_to_request_.size())(" potential matches between blocks ")(
            startHeight)(" and ")(highestTested.first)(" in ")(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now() - start)
                .count())(" milliseconds")
            .Flush();
        last_scanned_ = std::move(highestTested);
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Scan interrupted due to missing filter")
            .Flush();
    }
}

auto HDStateData::update_utxos(
    const block::bitcoin::Block& block,
    const block::Position& position,
    const block::Block::Matches matches) noexcept -> void
{
    auto transactions = std::map<
        block::pTxid,
        std::pair<
            std::vector<Bip32Index>,
            const block::bitcoin::Transaction*>>{};

    for (const auto& [txid, elementID] : matches) {
        const auto& [index, subchainID] = elementID;
        const auto& [subchain, accountID] = subchainID;
        const auto& element = node_.BalanceElement(subchain, index);
        const auto& pTransaction = block.at(txid->Bytes());

        OT_ASSERT(pTransaction);

        auto& arg = transactions[txid];
        auto& [outputs, pTX] = arg;
        const auto& transaction = *pTransaction;
        auto i = Bip32Index{0};

        for (const auto& output : transaction.Outputs()) {
            const auto& script = output.Script();

            switch (script.Type()) {
                case block::bitcoin::Script::Pattern::PayToPubkey: {
                    const auto pKey = element.Key();

                    OT_ASSERT(pKey);
                    OT_ASSERT(script.Pubkey().has_value());

                    const auto& key = *pKey;

                    if (key.PublicKey() == script.Pubkey().value()) {
                        outputs.emplace_back(i);

                        if (nullptr == pTX) { pTX = pTransaction.get(); }
                    }

                    // TODO mark key as used
                } break;
                case block::bitcoin::Script::Pattern::PayToPubkeyHash: {
                    const auto hash = element.PubkeyHash();

                    OT_ASSERT(script.PubkeyHash().has_value());

                    if (hash->Bytes() == script.PubkeyHash().value()) {
                        outputs.emplace_back(i);

                        if (nullptr == pTX) { pTX = pTransaction.get(); }
                    }

                    // TODO mark key as used
                } break;
                case block::bitcoin::Script::Pattern::PayToScriptHash:
                case block::bitcoin::Script::Pattern::PayToMultisig:
                default: {
                }
            };

            ++i;
        }
    }

    auto unspent = std::set<blockchain::block::bitcoin::Outpoint>{};

    for (const auto& utxo : db_.GetUnspentOutputs()) {
        unspent.insert(utxo.first);
    }

    for (const auto& transaction : block) {
        OT_ASSERT(transaction);

        for (const auto& input : transaction->Inputs()) {
            if (0 < unspent.count(input.PreviousOutput())) {
                auto& pTx = transactions[transaction->ID()].second;

                if (nullptr == pTx) { pTx = transaction.get(); }
            }
        }
    }

    for (const auto& [txid, data] : transactions) {
        auto& [outputs, pTX] = data;
        auto updated = db_.AddConfirmedTransaction(
            network_.Chain(),
            node_.ID(),
            subchain_,
            filter_type_,
            position,
            outputs,
            *pTX);

        OT_ASSERT(updated);  // TODO handle database errors
    }
}
}  // namespace opentxs::blockchain::client::implementation

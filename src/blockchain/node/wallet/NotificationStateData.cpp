// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/NotificationStateData.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/client/NymData.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/iterator/Bidirectional.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"  // IWYU pragma: keep
#include "opentxs/protobuf/HDPath.pb.h"

#define OT_METHOD "opentxs::blockchain::node::wallet::NotificationStateData::"

namespace opentxs::blockchain::node::wallet
{
NotificationStateData::NotificationStateData(
    const api::Core& api,
    const api::client::internal::Blockchain& crypto,
    const node::internal::Network& node,
    const WalletDatabase& db,
    const SimpleCallback& taskFinished,
    Outstanding& jobCounter,
    const filter::Type filter,
    const Type chain,
    const identifier::Nym& nym,
    OTPaymentCode&& code,
    proto::HDPath&& path) noexcept
    : SubchainStateData(
          api,
          crypto,
          node,
          db,
          OTNymID{nym},
          crypto::SubaccountType::Notification,
          calculate_id(api, chain, code),
          taskFinished,
          jobCounter,
          filter,
          Subchain::Notification)
    , path_(std::move(path))
    , code_(std::move(code))
{
    init();
    auto reason =
        api_.Factory().PasswordPrompt("Verifying / updating contact data");
    auto mNym = api_.Wallet().mutable_Nym(nym, reason);
    const auto type = Translate(chain);
    const auto existing = mNym.PaymentCode(type);
    const auto expected = code_->asBase58();

    if (existing != expected) {
        mNym.AddPaymentCode(expected, type, existing.empty(), true, reason);
    }
}

auto NotificationStateData::calculate_id(
    const api::Core& api,
    const Type chain,
    const PaymentCode& code) noexcept -> OTIdentifier
{
    auto preimage = api.Factory().Data(code.ID().Bytes());
    preimage->Concatenate(&chain, sizeof(chain));
    auto output = api.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}

auto NotificationStateData::check_index() noexcept -> bool
{
    const auto index = db_.SubchainLastIndexed(index_);

    if (index.value_or(0) < code_->Version()) {
        LogVerbose(OT_METHOD)(__func__)(": Payment code ")(code_->asBase58())(
            " notification elements not yet indexed")
            .Flush();
        static constexpr auto job{"index"};

        return queue_work(Task::index, job);
    } else {
        LogTrace(OT_METHOD)(__func__)(": Payment code ")(code_->asBase58())(
            " already indexed")
            .Flush();
    }

    return false;
}

auto NotificationStateData::handle_confirmed_matches(
    const block::bitcoin::Block& block,
    const block::Position& position,
    const block::Block::Matches& confirmed) noexcept -> void
{
    const auto& [utxo, general] = confirmed;
    LogVerbose(OT_METHOD)(__func__)(": ")(general.size())(
        " confirmed matches for ")(code_->asBase58())(" on ")(
        DisplayString(node_.Chain()))
        .Flush();

    if (0u == general.size()) { return; }

    const auto reason = init_keys();

    for (const auto& match : general) {
        const auto& [txid, elementID] = match;
        const auto& [version, subchainID] = elementID;
        LogVerbose(OT_METHOD)(__func__)(": ")(DisplayString(node_.Chain()))(
            " transaction ")(txid->asHex())(" contains a version ")(
            version)(" notification for ")(code_->asBase58())
            .Flush();
        const auto tx = block.at(txid->Bytes());

        OT_ASSERT(tx);

        process(match, *tx, reason);
    }
}

auto NotificationStateData::handle_mempool_matches(
    const block::Block::Matches& matches,
    std::unique_ptr<const block::bitcoin::Transaction> tx) noexcept -> void
{
    const auto& [utxo, general] = matches;

    if (0u == general.size()) { return; }

    const auto reason = init_keys();

    for (const auto& match : general) {
        const auto& [txid, elementID] = match;
        const auto& [version, subchainID] = elementID;
        LogVerbose(OT_METHOD)(__func__)(": ")(DisplayString(node_.Chain()))(
            " mempool transaction ")(txid->asHex())(" contains a version ")(
            version)(" notification for ")(code_->asBase58())
            .Flush();
        process(match, *tx, reason);
    }
}

auto NotificationStateData::index() noexcept -> void
{
    auto elements = WalletDatabase::ElementMap{};

    for (auto i{code_->Version()}; i > 0; --i) {
        auto& vector = elements[i];

        switch (i) {
            case 1:
            case 2: {
                code_->Locator(writer(vector.emplace_back()), i);
            } break;
            case 3:
            default: {
                vector.reserve(2);
                auto b = std::array<std::byte, 33>{};
                auto& prefix = b[0];
                auto* start = std::next(b.data(), 1);
                auto* stop = std::next(b.data(), b.size());
                code_->Locator(preallocated(32, start), i);
                prefix = std::byte{0x02};
                vector.emplace_back(b.data(), stop);
                prefix = std::byte{0x03};
                vector.emplace_back(b.data(), stop);
            }
        }
    }

    db_.SubchainAddElements(index_, elements);
    LogTrace(OT_METHOD)(__func__)(": Payment code ")(code_->asBase58())(
        " indexed")
        .Flush();
}

auto NotificationStateData::init_keys() noexcept -> OTPasswordPrompt
{
    const auto reason = api_.Factory().PasswordPrompt(
        "Decoding payment code notification transaction");

    if (auto key{code_->Key()}; key) {
        if (false == key->HasPrivate()) {
            auto seed{path_.root()};
            const auto upgraded =
                code_->AddPrivateKeys(seed, *path_.child().rbegin(), reason);

            if (false == upgraded) { OT_FAIL; }
        }
    } else {
        OT_FAIL;
    }

    return reason;
}

auto NotificationStateData::process(
    const block::Block::Match match,
    const block::bitcoin::Transaction& tx,
    const PasswordPrompt& reason) noexcept -> void
{
    const auto& [txid, elementID] = match;
    const auto& [version, subchainID] = elementID;

    for (const auto& output : tx.Outputs()) {
        const auto& script = output.Script();

        if (script.IsNotification(version, code_)) {
            const auto elements = [&] {
                auto out = PaymentCode::Elements{};

                for (auto i{0u}; i < 3u; ++i) {
                    const auto view = script.MultisigPubkey(i);

                    OT_ASSERT(view.has_value());

                    const auto& value = view.value();
                    auto* start =
                        reinterpret_cast<const std::byte*>(value.data());
                    auto* stop = std::next(start, value.size());
                    out.emplace_back(start, stop);
                }

                return out;
            }();

            auto pSender =
                code_->DecodeNotificationElements(version, elements, reason);

            if (!pSender) { continue; }

            const auto& sender = *pSender;
            LogVerbose(OT_METHOD)(__func__)(": decoded incoming notification "
                                            "from ")(sender.asBase58())(" on ")(
                DisplayString(node_.Chain()))(" for ")(code_->asBase58())
                .Flush();
            const auto& account = crypto_.PaymentCodeSubaccount(
                owner_, code_, sender, path_, node_.Chain(), reason);
            LogVerbose(OT_METHOD)(__func__)(": Created new account ")(
                account.ID())
                .Flush();
        }
    }
}

auto NotificationStateData::type() const noexcept -> std::stringstream
{
    auto output = std::stringstream{};
    output << "Payment code notification";

    return output;
}
}  // namespace opentxs::blockchain::node::wallet

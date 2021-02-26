// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/client/wallet/Wallet.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <optional>
#include <sstream>
#include <stdexcept>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/blockchain/BalanceNode.hpp"
#include "opentxs/api/client/blockchain/PaymentCode.hpp"
#include "opentxs/api/client/blockchain/Subchain.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/protobuf/BlockchainOutputMultisigDetails.pb.h"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"
#include "opentxs/protobuf/BlockchainTransactionProposedNotification.pb.h"
#include "opentxs/protobuf/BlockchainTransactionProposedOutput.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/HDPath.pb.h"
#include "util/ScopeGuard.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::client::implementation::Wallet::Proposals::"         \
    "BitcoinTransactionBuilder::"

namespace be = boost::endian;

namespace opentxs::blockchain::client::implementation
{
const std::size_t
    Wallet::Proposals::BitcoinTransactionBuilder::p2pkh_output_bytes_{34};

Wallet::Proposals::BitcoinTransactionBuilder::BitcoinTransactionBuilder(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const internal::WalletDatabase& db,
    const Identifier& proposal,
    const Type chain,
    const Amount feeRate) noexcept
    : api_(api)
    , blockchain_(blockchain)
    , db_(db)
    , proposal_(proposal)
    , chain_(chain)
    , fee_rate_(feeRate)
    , version_(1)
    , lock_time_(0xFFFFFFFF)
    , outputs_()
    , change_()
    , inputs_()
    , fixed_overhead_(sizeof(version_) + sizeof(lock_time_))
    , input_count_()
    , output_count_()
    , input_total_()
    , output_total_()
    , input_value_()
    , output_value_()
{
}

auto Wallet::Proposals::BitcoinTransactionBuilder::add_signatures(
    const ReadView preimage,
    const blockchain::bitcoin::SigHash& sigHash,
    block::bitcoin::internal::Input& input) const noexcept -> bool
{
    const auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    OT_ASSERT(0 < input.Keys().size());

    auto keys = std::vector<OTData>{};
    auto signatures = std::vector<Space>{};
    auto views = block::bitcoin::internal::Input::Signatures{};

    for (const auto& id : input.Keys()) {
        const auto& node = blockchain_.GetKey(id);
        const auto pKey = node.PrivateKey(reason);

        OT_ASSERT(pKey);

        const auto& key = *pKey;
        const auto& pubkey =
            keys.emplace_back(api_.Factory().Data(key.PublicKey()));
        auto& sig = signatures.emplace_back();
        sig.reserve(80);
        const auto haveSig = key.SignDER(preimage, hash_type(), sig, reason);

        if (false == haveSig) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to obtain signature")
                .Flush();

            return false;
        }

        sig.emplace_back(sigHash.flags_);

        OT_ASSERT(0 < key.PublicKey().size());

        views.emplace_back(reader(sig), pubkey->Bytes());
    }

    if (false == input.AddSignatures(views)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to apply signature")
            .Flush();

        return false;
    }

    return true;
}

auto Wallet::Proposals::BitcoinTransactionBuilder::AddChange(
    const Proposal& data) noexcept -> bool
{
    try {
        const auto reservedKey = db_.ReserveChangeKey(proposal_);

        if (false == reservedKey.has_value()) {
            throw std::runtime_error{"Failed to reserve change key"};
        }

        auto pOutput = [&] {
            const auto& keyID = reservedKey.value();
            const auto& element = blockchain_.GetKey(keyID);
            auto elements = [&] {
                namespace bb = opentxs::blockchain::block::bitcoin;
                namespace bi = bb::internal;
                auto out = bb::ScriptElements{};

                if (const auto size{data.notification().size()}; 1 < size) {
                    throw std::runtime_error{
                        "Multiple notifications not yet supported"};
                } else if (1 == size) {
                    const auto& notif = data.notification(0);
                    const auto recipient =
                        api_.Factory().PaymentCode(notif.recipient());
                    const auto message =
                        std::string{
                            "Constructing notification transaction to "} +
                        recipient->asBase58();
                    const auto reason = api_.Factory().PasswordPrompt(message);
                    const auto pc = [&] {
                        auto out = api_.Factory().PaymentCode(notif.sender());
                        const auto& path = notif.path();
                        auto seed{path.root()};
                        const auto rc = out->AddPrivateKeys(
                            seed, *path.child().rbegin(), reason);

                        if (false == rc) {
                            throw std::runtime_error{
                                "Failed to load private keys"};
                        }

                        return out;
                    }();
                    const auto pKey = element.PrivateKey(reason);

                    if (!pKey) {
                        throw std::runtime_error{
                            "Failed to load private change key"};
                    }

                    const auto& key = *pKey;
                    const auto keys = pc->GenerateNotificationElements(
                        recipient, key, reason);

                    if (3u != keys.size()) {
                        throw std::runtime_error{
                            "Failed to obtain notification elements"};
                    }

                    out.emplace_back(bi::Opcode(bb::OP::ONE));
                    out.emplace_back(bi::PushData(reader(keys.at(0))));
                    out.emplace_back(bi::PushData(reader(keys.at(1))));
                    out.emplace_back(bi::PushData(reader(keys.at(2))));
                    out.emplace_back(bi::Opcode(bb::OP::THREE));
                    out.emplace_back(bi::Opcode(bb::OP::CHECKMULTISIG));
                } else {
                    const auto pkh = element.PubkeyHash();
                    out.emplace_back(bi::Opcode(bb::OP::DUP));
                    out.emplace_back(bi::Opcode(bb::OP::HASH160));
                    out.emplace_back(bi::PushData(pkh->Bytes()));
                    out.emplace_back(bi::Opcode(bb::OP::EQUALVERIFY));
                    out.emplace_back(bi::Opcode(bb::OP::CHECKSIG));
                }

                return out;
            }();
            auto pScript = factory::BitcoinScript(chain_, std::move(elements));

            if (false == bool(pScript)) {
                throw std::runtime_error{"Failed to construct script"};
            }

            return factory::BitcoinTransactionOutput(
                api_,
                blockchain_,
                chain_,
                outputs_.size(),
                0,
                std::move(pScript),
                {keyID});
        }();

        if (false == bool(pOutput)) {
            throw std::runtime_error{"Failed to construct output"};
        }

        {
            auto& output = *pOutput;
            output_value_ += output.Value();
            output_total_ += output.CalculateSize();

            OT_ASSERT(0 < output.Keys().size());
        }

        change_.emplace_back(std::move(pOutput));
        output_count_ = outputs_.size() + change_.size();

        return true;
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        return false;
    }
}

auto Wallet::Proposals::BitcoinTransactionBuilder::AddInput(
    const UTXO& utxo) noexcept -> bool
{
    auto pInput =
        factory::BitcoinTransactionInput(api_, blockchain_, chain_, utxo);

    if (false == bool(pInput)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct input")
            .Flush();

        return false;
    }

    const auto& input = *pInput;
    input_count_ = inputs_.size();
    input_total_ += input.CalculateSize();
    const auto amount = static_cast<Amount>(utxo.second.value());
    input_value_ += amount;
    inputs_.emplace_back(std::move(pInput), amount);

    return true;
}

auto Wallet::Proposals::BitcoinTransactionBuilder::bip_69() noexcept -> void
{
    auto inputSort = [](const auto& lhs, const auto& rhs) -> auto
    {
        return lhs.first->PreviousOutput() < rhs.first->PreviousOutput();
    };
    auto outputSort = [](const auto& lhs, const auto& rhs) -> auto
    {
        if (lhs->Value() == rhs->Value()) {
            auto lScript = Space{};
            auto rScript = Space{};
            lhs->Script().Serialize(writer(lScript));
            rhs->Script().Serialize(writer(rScript));

            return std::lexicographical_compare(
                std::begin(lScript),
                std::end(lScript),
                std::begin(rScript),
                std::end(rScript));
        } else {

            return lhs->Value() < rhs->Value();
        }
    };

    std::sort(std::begin(inputs_), std::end(inputs_), inputSort);
    std::sort(std::begin(outputs_), std::end(outputs_), outputSort);
    auto index{-1};

    for (const auto& output : outputs_) { output->SetIndex(++index); }
}

auto Wallet::Proposals::BitcoinTransactionBuilder::bytes() const noexcept
    -> std::size_t
{
    // NOTE assumes one additional output to account for change
    const auto outputs = bitcoin::CompactSize{output_count_.Value() + 1};

    return fixed_overhead_ + input_count_.Size() + input_total_ +
           outputs.Size() + output_total_ + p2pkh_output_bytes_;
}

auto Wallet::Proposals::BitcoinTransactionBuilder::CreateOutputs(
    const Proposal& proposal) noexcept -> bool
{
    namespace bb = opentxs::blockchain::block::bitcoin;
    namespace bi = bb::internal;

    auto index = std::int32_t{-1};

    for (const auto& output : proposal.output()) {
        auto pScript = std::unique_ptr<bi::Script>{};

        if (output.has_raw()) {
            pScript = factory::BitcoinScript(chain_, output.raw());
        } else {
            auto elements = bb::ScriptElements{};

            if (output.has_pubkeyhash()) {
                elements.emplace_back(bi::Opcode(bb::OP::DUP));
                elements.emplace_back(bi::Opcode(bb::OP::HASH160));
                elements.emplace_back(bi::PushData(output.pubkeyhash()));
                elements.emplace_back(bi::Opcode(bb::OP::EQUALVERIFY));
                elements.emplace_back(bi::Opcode(bb::OP::CHECKSIG));
            } else if (output.has_scripthash()) {
                elements.emplace_back(bi::Opcode(bb::OP::HASH160));
                elements.emplace_back(bi::PushData(output.scripthash()));
                elements.emplace_back(bi::Opcode(bb::OP::EQUAL));
            } else if (output.has_pubkey()) {
                elements.emplace_back(bi::PushData(output.pubkey()));
                elements.emplace_back(bi::Opcode(bb::OP::CHECKSIG));
            } else if (output.has_multisig()) {
                const auto& ms = output.multisig();
                const auto M = static_cast<std::uint8_t>(ms.m());
                const auto N = static_cast<std::uint8_t>(ms.n());
                elements.emplace_back(bi::Opcode(static_cast<bb::OP>(M + 80)));

                for (const auto& key : ms.pubkey()) {
                    elements.emplace_back(bi::PushData(key));
                }

                elements.emplace_back(bi::Opcode(static_cast<bb::OP>(N + 80)));
                elements.emplace_back(bi::Opcode(bb::OP::CHECKMULTISIG));
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported output type")
                    .Flush();

                return false;
            }

            pScript = factory::BitcoinScript(chain_, std::move(elements));
        }

        if (false == bool(pScript)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct script")
                .Flush();

            return false;
        }

        auto pOutput = factory::BitcoinTransactionOutput(
            api_,
            blockchain_,
            chain_,
            static_cast<std::uint32_t>(++index),
            static_cast<std::int64_t>(output.amount()),
            std::move(pScript),
            {});

        if (false == bool(pOutput)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct output")
                .Flush();

            return false;
        }

        if (output.has_paymentcodechannel()) {
            try {
                const auto accountID =
                    api_.Factory().Identifier(output.paymentcodechannel());
                const auto& account = blockchain_.PaymentCodeSubaccount(
                    blockchain_.Owner(accountID), accountID);
                static constexpr auto subchain{
                    api::client::blockchain::Subchain::Outgoing};
                const auto& element = account.BalanceElement(subchain, 0);
                pOutput->SetPayee(element.Contact());
            } catch (const std::exception& e) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
            }
        }

        output_value_ += pOutput->Value();
        output_total_ += pOutput->CalculateSize();
        outputs_.emplace_back(std::move(pOutput));
    }

    output_count_ = outputs_.size() + change_.size();

    return true;
}

auto Wallet::Proposals::BitcoinTransactionBuilder::dust() const noexcept
    -> std::size_t
{
    // TODO this should account for script type

    return std::size_t{148} * fee_rate_ / 1000;
}

auto Wallet::Proposals::BitcoinTransactionBuilder::FinalizeOutputs() noexcept
    -> void
{
    OT_ASSERT(IsFunded());

    const auto excessValue = input_value_ - (output_value_ + required_fee());

    if (excessValue <= dust()) {
        for (const auto& output : change_) {
            for (const auto& key : output->Keys()) {
                db_.ReleaseChangeKey(proposal_, key);
            }
        }
    } else {
        OT_ASSERT(1 == change_.size());  // TODO

        auto& change = *change_.begin();
        change->SetValue(excessValue);
        output_value_ += change->Value();
        outputs_.emplace_back(std::move(change));
    }

    change_.clear();
    bip_69();
}

auto Wallet::Proposals::BitcoinTransactionBuilder::
    FinalizeTransaction() noexcept -> Transaction
{
    LogTrace(OT_METHOD)(__FUNCTION__)(": ")(print()).Flush();
    auto inputs = factory::BitcoinTransactionInputs([&] {
        auto output = std::vector<Input>{};
        output.reserve(inputs_.size());

        for (auto& [input, value] : inputs_) {
            output.emplace_back(std::move(input));
        }

        return output;
    }());

    if (false == bool(inputs)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct inputs")
            .Flush();

        return {};
    }

    auto outputs = factory::BitcoinTransactionOutputs(std::move(outputs_));

    if (false == bool(outputs)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct outputs")
            .Flush();

        return {};
    }

    return factory::BitcoinTransaction(
        api_,
        blockchain_,
        chain_,
        Clock::now(),
        version_,
        lock_time_,
        std::move(inputs),
        std::move(outputs));
}

auto Wallet::Proposals::BitcoinTransactionBuilder::get_single(
    const std::size_t index,
    const blockchain::bitcoin::SigHash& sigHash) const noexcept
    -> std::unique_ptr<Hash>
{
    if (bitcoin::SigOption::Single != sigHash.Type()) { return nullptr; }

    if (index >= outputs_.size()) { return nullptr; }

    // TODO not implemented yet

    return nullptr;
}

auto Wallet::Proposals::BitcoinTransactionBuilder::hash_type() const noexcept
    -> proto::HashType
{
    return proto::HASHTYPE_SHA256D;
}

auto Wallet::Proposals::BitcoinTransactionBuilder::init_bip143(
    Bip143& bip143) const noexcept -> bool
{
    if (bip143.has_value()) { return true; }

    auto success{false};
    const auto postcondition = ScopeGuard{[&]() {
        if (false == success) { bip143 = std::nullopt; }
    }};
    bip143.emplace();

    OT_ASSERT(bip143.has_value());

    auto& output = bip143.value();
    auto cb = [&](const auto& preimage, auto& output) -> bool {
        return api_.Crypto().Hash().Digest(
            proto::HASHTYPE_SHA256D,
            reader(preimage),
            preallocated(output.size(), output.data()));
    };

    {
        auto preimage =
            space(inputs_.size() * sizeof(block::bitcoin::Outpoint));
        auto it = preimage.data();

        for (const auto& [input, amount] : inputs_) {
            const auto& outpoint = input->PreviousOutput();
            std::memcpy(it, &outpoint, sizeof(outpoint));
            std::advance(it, sizeof(outpoint));
        }

        if (false == cb(preimage, output.outpoints_)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to hash outpoints")
                .Flush();

            return false;
        }
    }

    {
        auto preimage = space(inputs_.size() * sizeof(std::uint32_t));
        auto it = preimage.data();

        for (const auto& [input, value] : inputs_) {
            const auto sequence = input->Sequence();
            std::memcpy(it, &sequence, sizeof(sequence));
            std::advance(it, sizeof(sequence));
        }

        if (false == cb(preimage, output.sequences_)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to hash sequences")
                .Flush();

            return false;
        }
    }

    {
        auto preimage = space(output_total_);
        auto it = preimage.data();

        for (const auto& output : outputs_) {
            const auto size = output->CalculateSize();

            if (false ==
                output->Serialize(preallocated(size, it)).has_value()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to serialize output")
                    .Flush();

                return false;
            }

            std::advance(it, size);
        }

        if (false == cb(preimage, output.outputs_)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to hash outputs")
                .Flush();

            return false;
        }
    }

    success = true;

    return true;
}

auto Wallet::Proposals::BitcoinTransactionBuilder::init_txcopy(
    Transaction& txcopy) const noexcept -> bool
{
    if (txcopy) { return true; }

    auto inputCopy = std::vector<Input>{};
    std::transform(
        std::begin(inputs_), std::end(inputs_), std::back_inserter(inputCopy), [
        ](const auto& input) -> auto {
            return input.first->SignatureVersion();
        });
    auto inputs = factory::BitcoinTransactionInputs(std::move(inputCopy));

    if (false == bool(inputs)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct inputs")
            .Flush();

        return {};
    }

    auto outputCopy = std::vector<Output>{};
    std::transform(
        std::begin(outputs_),
        std::end(outputs_),
        std::back_inserter(outputCopy),
        [](const auto& output) -> auto { return output->clone(); });
    auto outputs = factory::BitcoinTransactionOutputs(std::move(outputCopy));

    if (false == bool(outputs)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct outputs")
            .Flush();

        return {};
    }

    txcopy = factory::BitcoinTransaction(
        api_,
        blockchain_,
        chain_,
        Clock::now(),
        version_,
        lock_time_,
        std::move(inputs),
        std::move(outputs));

    return bool(txcopy);
}

auto Wallet::Proposals::BitcoinTransactionBuilder::is_segwit(
    const block::bitcoin::internal::Input& input) noexcept -> bool
{
    // TODO

    return false;
}

auto Wallet::Proposals::BitcoinTransactionBuilder::IsFunded() const noexcept
    -> bool
{
    return input_value_ > (output_value_ + required_fee());
}

auto Wallet::Proposals::BitcoinTransactionBuilder::print() const noexcept
    -> std::string
{
    auto text = std::stringstream{};
    text << "\n     version: " << std::to_string(version_.value()) << '\n';
    text << "   lock time: " << std::to_string(lock_time_.value()) << '\n';
    text << " input count: " << std::to_string(inputs_.size()) << '\n';

    for (const auto& [input, value] : inputs_) {
        const auto& outpoint = input->PreviousOutput();
        text << " * " << outpoint.str()
             << ", sequence: " << std::to_string(input->Sequence())
             << ", value: " << std::to_string(value) << '\n';
    }

    text << "output count: " << std::to_string(outputs_.size()) << '\n';

    for (const auto& output : outputs_) {
        text << " * bytes: " << std::to_string(output->CalculateSize())
             << ", value: " << std::to_string(output->Value()) << '\n';
    }

    text << "total output value: " << std::to_string(output_value_) << '\n';
    text << " total input value: " << std::to_string(input_value_) << '\n';
    text << "               fee: "
         << std::to_string(input_value_ - output_value_);

    return text.str();
}

auto Wallet::Proposals::BitcoinTransactionBuilder::required_fee() const noexcept
    -> Amount
{
    return (bytes() * fee_rate_) / 1000;
}

auto Wallet::Proposals::BitcoinTransactionBuilder::sign_input(
    const int index,
    block::bitcoin::internal::Input& input,
    Transaction& txcopy,
    Bip143& bip143) const noexcept -> bool
{
    switch (chain_) {
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3: {

            return sign_input_bch(index, input, bip143);
        }
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest: {
            if (is_segwit(input)) {
                // TODO not implemented yet

                return false;
            }

            return sign_input_btc(index, input, txcopy);
        }
        case Type::Unknown:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported chain").Flush();

            return false;
        }
    }
}

auto Wallet::Proposals::BitcoinTransactionBuilder::sign_input_bch(
    const int index,
    block::bitcoin::internal::Input& input,
    Bip143& bip143) const noexcept -> bool
{
    if (false == init_bip143(bip143)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error instantiating txcopy")
            .Flush();

        return false;
    }

    const auto sigHash = blockchain::bitcoin::SigHash{chain_};
    const auto& outpoints = bip143->Outpoints(sigHash);
    const auto& sequences = bip143->Sequences(sigHash);
    const auto& outpoint = input.PreviousOutput();
    const auto pScript = input.Spends().SigningSubscript();

    OT_ASSERT(pScript);

    const auto& script = *pScript;
    const auto scriptBytes = script.CalculateSize();
    const auto cs = blockchain::bitcoin::CompactSize{scriptBytes};
    const auto& output = input.Spends();
    const auto value = output.Value();
    const auto sequence = input.Sequence();
    const auto single = get_single(index, sigHash);
    const auto& outputs = bip143->Outputs(sigHash, single.get());
    // clang-format off
    auto preimage = space(
        sizeof(version_) +
        sizeof(outpoints) +
        sizeof(sequences) +
        sizeof(outpoint) +
        cs.Total() +
        sizeof(value) +
        sizeof(sequence) +
        sizeof(outputs) +
        sizeof(lock_time_) +
        sizeof(sigHash));
    // clang-format on
    auto it = preimage.data();
    std::memcpy(it, &version_, sizeof(version_));
    std::advance(it, sizeof(version_));
    std::memcpy(it, outpoints.data(), outpoints.size());
    std::advance(it, outpoints.size());
    std::memcpy(it, sequences.data(), sequences.size());
    std::advance(it, sequences.size());
    std::memcpy(it, &outpoint, sizeof(outpoint));
    std::advance(it, sizeof(outpoint));

    if (false == cs.Encode(preallocated(cs.Size(), it))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": CompactSize encoding failure")
            .Flush();

        return false;
    }

    std::advance(it, cs.Size());

    if (false == script.Serialize(preallocated(scriptBytes, it))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Script encoding failure").Flush();

        return false;
    }

    std::advance(it, scriptBytes);
    std::memcpy(it, &value, sizeof(value));
    std::advance(it, sizeof(value));
    std::memcpy(it, &sequence, sizeof(sequence));
    std::advance(it, sizeof(sequence));
    std::memcpy(it, outputs.data(), outputs.size());
    std::advance(it, outputs.size());
    std::memcpy(it, &lock_time_, sizeof(lock_time_));
    std::advance(it, sizeof(lock_time_));
    std::memcpy(it, &sigHash, sizeof(sigHash));
    std::advance(it, sizeof(sigHash));

    return add_signatures(reader(preimage), sigHash, input);
}

auto Wallet::Proposals::BitcoinTransactionBuilder::sign_input_btc(
    const int index,
    block::bitcoin::internal::Input& input,
    Transaction& txcopy) const noexcept -> bool
{
    if (false == init_txcopy(txcopy)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error instantiating txcopy")
            .Flush();

        return false;
    }

    const auto sigHash = blockchain::bitcoin::SigHash{chain_};
    auto preimage = txcopy->GetPreimageBTC(index, sigHash);

    if (0 == preimage.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error obtaining signing preimage")
            .Flush();

        return false;
    }

    std::copy(sigHash.begin(), sigHash.end(), std::back_inserter(preimage));

    return add_signatures(reader(preimage), sigHash, input);
}

auto Wallet::Proposals::BitcoinTransactionBuilder::SignInputs() noexcept -> bool
{
    auto index = int{-1};
    auto txcopy = Transaction{};
    auto bip143 = std::optional<bitcoin::Bip143Hashes>{};

    for (const auto& [input, value] : inputs_) {
        if (false == sign_input(++index, *input, txcopy, bip143)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign input")
                .Flush();

            return false;
        }
    }

    return true;
}
}  // namespace opentxs::blockchain::client::implementation

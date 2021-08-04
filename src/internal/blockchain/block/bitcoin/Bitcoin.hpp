// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

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
namespace bitcoin
{
class Inventory;
struct EncodedInput;
struct EncodedOutpoint;
struct EncodedOutput;
struct EncodedTransaction;
}  // namespace bitcoin

namespace block
{
class Header;

namespace bitcoin
{
namespace internal
{
struct Header;
struct Output;
struct Script;
}  // namespace internal

class Block;
class Header;
class Input;
class Inputs;
class Output;
class Outputs;
class Script;
class Transaction;
}  // namespace bitcoin

struct Outpoint;
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace blockchain
{
namespace bitcoin
{
class CompactSize;
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network

namespace proto
{
class BlockchainBlockHeader;
class BlockchainTransaction;
class BlockchainTransactionInput;
class BlockchainTransactionOutput;
}  // namespace proto

class Identifier;
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::internal
{
auto DecodeBip34(const ReadView coinbase) noexcept -> block::Height;
auto EncodeBip34(block::Height height) noexcept -> Space;

struct Input : virtual public bitcoin::Input {
    using Signature = std::pair<ReadView, ReadView>;
    using Signatures = std::vector<Signature>;

    virtual auto AssociatedLocalNyms(
        const api::client::Blockchain& blockchain,
        std::vector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        const api::client::Blockchain& blockchain,
        std::vector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Input> = 0;
    virtual auto NetBalanceChange(
        const api::client::Blockchain& blockchain,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount = 0;
    virtual auto SignatureVersion() const noexcept
        -> std::unique_ptr<Input> = 0;
    virtual auto SignatureVersion(std::unique_ptr<internal::Script> subscript)
        const noexcept -> std::unique_ptr<Input> = 0;
    virtual auto Spends() const noexcept(false) -> const Output& = 0;

    virtual auto AddMultisigSignatures(const Signatures& signatures) noexcept
        -> bool = 0;
    virtual auto AddSignatures(const Signatures& signatures) noexcept
        -> bool = 0;
    virtual auto AssociatePreviousOutput(
        const api::client::Blockchain& api,
        const proto::BlockchainTransactionOutput& output) noexcept -> bool = 0;
    virtual auto MergeMetadata(
        const api::client::Blockchain& api,
        const SerializeType& rhs) noexcept -> void = 0;
    virtual auto ReplaceScript() noexcept -> bool = 0;

    ~Input() override = default;
};
struct Inputs : virtual public bitcoin::Inputs {
    virtual auto AssociatedLocalNyms(
        const api::client::Blockchain& blockchain,
        std::vector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        const api::client::Blockchain& blockchain,
        std::vector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Inputs> = 0;
    virtual auto NetBalanceChange(
        const api::client::Blockchain& blockchain,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount = 0;

    virtual auto AnyoneCanPay(const std::size_t index) noexcept -> bool = 0;
    virtual auto AssociatePreviousOutput(
        const api::client::Blockchain& api,
        const std::size_t inputIndex,
        const proto::BlockchainTransactionOutput& output) noexcept -> bool = 0;
    virtual auto MergeMetadata(
        const api::client::Blockchain& api,
        const Input::SerializeType& rhs) noexcept(false) -> void = 0;
    virtual auto ReplaceScript(const std::size_t index) noexcept -> bool = 0;

    ~Inputs() override = default;
};
struct Output : virtual public bitcoin::Output {
    virtual auto AssociatedLocalNyms(
        const api::client::Blockchain& blockchain,
        std::vector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        const api::client::Blockchain& blockchain,
        std::vector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Output> = 0;
    virtual auto NetBalanceChange(
        const api::client::Blockchain& blockchain,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount = 0;
    virtual auto SigningSubscript() const noexcept
        -> std::unique_ptr<internal::Script> = 0;

    virtual auto ForTestingOnlyAddKey(const KeyID& key) noexcept -> void = 0;
    virtual auto MergeMetadata(const SerializeType& rhs) noexcept -> void = 0;
    virtual auto SetIndex(const std::uint32_t index) noexcept -> void = 0;
    virtual auto SetValue(const std::uint64_t value) noexcept -> void = 0;
    virtual auto SetPayee(const Identifier& contact) noexcept -> void = 0;
    virtual auto SetPayer(const Identifier& contact) noexcept -> void = 0;

    ~Output() override = default;
};
struct Outputs : virtual public bitcoin::Outputs {
    virtual auto AssociatedLocalNyms(
        const api::client::Blockchain& blockchain,
        std::vector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        const api::client::Blockchain& blockchain,
        std::vector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Outputs> = 0;
    virtual auto NetBalanceChange(
        const api::client::Blockchain& blockchain,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount = 0;

    virtual auto ForTestingOnlyAddKey(
        const std::size_t index,
        const blockchain::crypto::Key& key) noexcept -> bool = 0;
    virtual auto MergeMetadata(const Output::SerializeType& rhs) noexcept(false)
        -> void = 0;

    ~Outputs() override = default;
};
struct Script : virtual public bitcoin::Script {
    static auto blank_signature(const blockchain::Type chain) noexcept
        -> const Space&;
    static auto blank_pubkey(
        const blockchain::Type chain,
        const bool compressed = true) noexcept -> const Space&;

    virtual auto clone() const noexcept -> std::unique_ptr<Script> = 0;
    virtual auto LikelyPubkeyHashes(const api::Core& api) const noexcept
        -> std::vector<OTData> = 0;
    virtual auto SigningSubscript(const blockchain::Type chain) const noexcept
        -> std::unique_ptr<Script> = 0;

    ~Script() override = default;
};
struct Transaction : virtual public bitcoin::Transaction {
    using SigHash = blockchain::bitcoin::SigOption;

    virtual auto GetPreimageBTC(
        const std::size_t index,
        const blockchain::bitcoin::SigHash& hashType) const noexcept
        -> Space = 0;

    virtual auto AssociatePreviousOutput(
        const api::client::Blockchain& api,
        const std::size_t inputIndex,
        const proto::BlockchainTransactionOutput& output) noexcept -> bool = 0;
    virtual auto ForTestingOnlyAddKey(
        const std::size_t index,
        const blockchain::crypto::Key& key) noexcept -> bool = 0;
    virtual auto MergeMetadata(
        const api::client::Blockchain& api,
        const blockchain::Type chain,
        const SerializeType& rhs) noexcept -> void = 0;
    virtual auto SetMemo(const std::string& memo) noexcept -> void = 0;

    ~Transaction() override = default;
};
}  // namespace opentxs::blockchain::block::bitcoin::internal

namespace opentxs::factory
{
#if OT_BLOCKCHAIN
using UTXO =
    std::pair<blockchain::block::Outpoint, proto::BlockchainTransactionOutput>;
using Transaction_p =
    std::shared_ptr<const opentxs::blockchain::block::bitcoin::Transaction>;
using AbortFunction = std::function<bool()>;

auto BitcoinBlock(
    const api::Core& api,
    const opentxs::blockchain::block::Header& previous,
    const Transaction_p generationTransaction,
    const std::uint32_t nBits,
    const std::vector<Transaction_p>& extraTransactions,
    const std::int32_t version,
    const AbortFunction abort) noexcept
    -> std::shared_ptr<const opentxs::blockchain::block::bitcoin::Block>;
auto BitcoinBlock(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const ReadView in) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Block>;
auto BitcoinBlockHeader(
    const api::Core& api,
    const opentxs::blockchain::block::Header& previous,
    const std::uint32_t nBits,
    const std::int32_t version,
    opentxs::blockchain::block::pHash&& merkle,
    const AbortFunction abort) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto BitcoinBlockHeader(
    const api::Core& api,
    const proto::BlockchainBlockHeader& serialized) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto BitcoinBlockHeader(
    const api::Core& api,
    const blockchain::Type chain,
    const ReadView bytes) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto BitcoinBlockHeader(
    const api::Core& api,
    const blockchain::Type chain,
    const blockchain::block::Hash& hash,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto BitcoinScript(
    const blockchain::Type chain,
    const ReadView bytes,
    const blockchain::block::bitcoin::Script::Position role,
    const bool allowInvalidOpcodes = true,
    const bool mute = false) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Script>;
auto BitcoinScript(
    const blockchain::Type chain,
    blockchain::block::bitcoin::ScriptElements&& elements,
    const blockchain::block::bitcoin::Script::Position role) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Script>;
auto BitcoinTransaction(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const Time& time,
    const boost::endian::little_int32_buf_t& version,
    const boost::endian::little_uint32_buf_t lockTime,
    bool segwit,
    std::unique_ptr<blockchain::block::bitcoin::internal::Inputs> inputs,
    std::unique_ptr<blockchain::block::bitcoin::internal::Outputs>
        outputs) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>;
auto BitcoinTransaction(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    blockchain::bitcoin::EncodedTransaction&& parsed) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>;
auto BitcoinTransaction(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const proto::BlockchainTransaction& serialized) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>;
auto BitcoinTransactionInput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const ReadView outpoint,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool isGeneration,
    std::vector<Space>&& witness) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>;
auto BitcoinTransactionInput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const UTXO& spends,
    const std::optional<std::uint32_t> sequence = {}) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>;
auto BitcoinTransactionInput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const proto::BlockchainTransactionInput,
    const bool isGeneration) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>;
auto BitcoinTransactionInputs(
    std::vector<std::unique_ptr<blockchain::block::bitcoin::internal::Input>>&&
        inputs,
    std::optional<std::size_t> size = {}) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Inputs>;
auto BitcoinTransactionOutput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::uint32_t index,
    const std::uint64_t value,
    std::unique_ptr<const blockchain::block::bitcoin::internal::Script> script,
    const std::set<blockchain::crypto::Key>& keys) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>;
auto BitcoinTransactionOutput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::uint32_t index,
    const std::uint64_t value,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>;
auto BitcoinTransactionOutput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const proto::BlockchainTransactionOutput& in) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>;
auto BitcoinTransactionOutputs(
    std::vector<std::unique_ptr<blockchain::block::bitcoin::internal::Output>>&&
        outputs,
    std::optional<std::size_t> size = {}) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Outputs>;
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::factory

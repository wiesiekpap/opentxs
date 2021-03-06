// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "blockchain/bitcoin/CompactSize.hpp"
#include "blockchain/client/wallet/Account.hpp"
#include "blockchain/client/wallet/Accounts.hpp"
#include "blockchain/client/wallet/DeterministicStateData.hpp"
#include "blockchain/client/wallet/SubchainStateData.hpp"
#include "core/Worker.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"
#include "opentxs/protobuf/BlockchainTransactionProposal.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/util/WorkType.hpp"
#include "util/JobCounter.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal

class Blockchain;
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
struct Bip143Hashes;
struct SigHash;
}  // namespace bitcoin

namespace block
{
namespace bitcoin
{
namespace internal
{
struct Input;
struct Output;
struct Transaction;
}  // namespace internal

struct Outpoint;
}  // namespace bitcoin
}  // namespace block

namespace client
{
namespace implementation
{
class SubchainStateData;
}  // namespace implementation
}  // namespace client
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionOutput;
class BlockchainTransactionProposal;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace be = boost::endian;

namespace opentxs::blockchain::client::implementation
{
class Wallet final : virtual public internal::Wallet, Worker<Wallet, api::Core>
{
public:
    auto ConstructTransaction(const proto::BlockchainTransactionProposal& tx)
        const noexcept -> std::future<block::pTxid> final;

    auto Init() noexcept -> void final;
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return stop_worker();
    }

    Wallet(
        const api::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const internal::Network& parent,
        const internal::WalletDatabase& db,
        const Type chain,
        const std::string& shutdown) noexcept;

    ~Wallet() final;

private:
    friend Worker<Wallet, api::Core>;

    enum class Work : OTZMQWorkType {
        key = OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL,
        block = value(WorkType::BlockchainNewHeader),
        filter = OT_ZMQ_NEW_FILTER_SIGNAL,
        nym = value(WorkType::NymCreated),
        reorg = value(WorkType::BlockchainReorg),
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = value(WorkType::Shutdown),
    };

    struct Proposals {
        using Proposal = proto::BlockchainTransactionProposal;

        auto Add(const Proposal& tx) const noexcept
            -> std::future<block::pTxid>;

        auto Run() noexcept -> bool;

        Proposals(
            const api::Core& api,
            const api::client::Blockchain& blockchain,
            const internal::Network& network,
            const internal::WalletDatabase& db,
            const Type chain) noexcept;

    private:
        enum class BuildResult : std::int8_t {
            PermanentFailure = -1,
            Success = 0,
            TemporaryFailure = 1,
        };

        struct BitcoinTransactionBuilder {
            using UTXO = std::pair<
                blockchain::block::bitcoin::Outpoint,
                proto::BlockchainTransactionOutput>;
            using Transaction =
                std::unique_ptr<block::bitcoin::internal::Transaction>;

            auto IsFunded() const noexcept -> bool;

            auto AddChange(const Proposal& proposal) noexcept -> bool;
            auto AddInput(const UTXO& utxo) noexcept -> bool;
            auto CreateOutputs(const Proposal& proposal) noexcept -> bool;
            auto FinalizeOutputs() noexcept -> void;
            auto FinalizeTransaction() noexcept -> Transaction;
            auto SignInputs() noexcept -> bool;

            BitcoinTransactionBuilder(
                const api::Core& api,
                const api::client::Blockchain& blockchain,
                const internal::WalletDatabase& db,
                const Identifier& proposal,
                const Type chain,
                const Amount feeRate) noexcept;

        private:
            using Input = std::unique_ptr<block::bitcoin::internal::Input>;
            using Output = std::unique_ptr<block::bitcoin::internal::Output>;
            using Bip143 = std::optional<bitcoin::Bip143Hashes>;
            using Hash = std::array<std::byte, 32>;

            static const std::size_t p2pkh_output_bytes_;

            const api::Core& api_;
            const api::client::Blockchain& blockchain_;
            const internal::WalletDatabase& db_;
            const Identifier& proposal_;
            const Type chain_;
            const Amount fee_rate_;
            const be::little_int32_buf_t version_;
            const be::little_uint32_buf_t lock_time_;
            std::vector<Output> outputs_;
            std::vector<Output> change_;
            std::vector<std::pair<Input, Amount>> inputs_;
            const std::size_t fixed_overhead_;
            bitcoin::CompactSize input_count_;
            bitcoin::CompactSize output_count_;
            std::size_t input_total_;
            std::size_t output_total_;
            Amount input_value_;
            Amount output_value_;

            static auto is_segwit(
                const block::bitcoin::internal::Input& input) noexcept -> bool;

            auto add_signatures(
                const ReadView preimage,
                const blockchain::bitcoin::SigHash& sigHash,
                block::bitcoin::internal::Input& input) const noexcept -> bool;
            auto add_signatures_p2ms(
                const ReadView preimage,
                const blockchain::bitcoin::SigHash& sigHash,
                const PasswordPrompt& reason,
                const block::bitcoin::internal::Output& spends,
                block::bitcoin::internal::Input& input) const noexcept -> bool;
            auto add_signatures_p2pk(
                const ReadView preimage,
                const blockchain::bitcoin::SigHash& sigHash,
                const PasswordPrompt& reason,
                const block::bitcoin::internal::Output& spends,
                block::bitcoin::internal::Input& input) const noexcept -> bool;
            auto add_signatures_p2pkh(
                const ReadView preimage,
                const blockchain::bitcoin::SigHash& sigHash,
                const PasswordPrompt& reason,
                const block::bitcoin::internal::Output& spends,
                block::bitcoin::internal::Input& input) const noexcept -> bool;
            auto bytes() const noexcept -> std::size_t;
            auto dust() const noexcept -> std::size_t;
            auto get_single(
                const std::size_t index,
                const blockchain::bitcoin::SigHash& sigHash) const noexcept
                -> std::unique_ptr<Hash>;
            auto hash_type() const noexcept -> proto::HashType;
            auto init_bip143(Bip143& bip143) const noexcept -> bool;
            auto init_txcopy(Transaction& txcopy) const noexcept -> bool;
            auto print() const noexcept -> std::string;
            auto required_fee() const noexcept -> Amount;
            auto sign_input(
                const int index,
                block::bitcoin::internal::Input& input,
                Transaction& txcopy,
                Bip143& bip143) const noexcept -> bool;
            auto sign_input_bch(
                const int index,
                block::bitcoin::internal::Input& input,
                Bip143& bip143) const noexcept -> bool;
            auto sign_input_btc(
                const int index,
                block::bitcoin::internal::Input& input,
                Transaction& txcopy) const noexcept -> bool;

            auto bip_69() noexcept -> void;

            BitcoinTransactionBuilder() = delete;
        };

        using Builder = std::function<BuildResult(
            const Identifier& id,
            Proposal&,
            std::promise<block::pTxid>&)>;

        const api::Core& api_;
        const api::client::Blockchain& blockchain_;
        const internal::Network& network_;
        const internal::WalletDatabase& db_;
        const Type chain_;
        mutable std::mutex lock_;
        mutable std::map<OTIdentifier, std::promise<block::pTxid>> pending_;
        mutable std::map<OTIdentifier, Time> confirming_;

        static auto is_expired(const Proposal& tx) noexcept -> bool;

        auto build_transaction_bitcoin(
            const Identifier& id,
            Proposal& proposal,
            std::promise<block::pTxid>& promise) const noexcept -> BuildResult;
        auto cleanup(const Lock& lock) noexcept -> void;
        auto get_builder() const noexcept -> Builder;
        auto rebroadcast(const Lock& lock) noexcept -> void;
        auto send(const Lock& lock) noexcept -> void;
    };

    const internal::Network& parent_;
    const internal::WalletDatabase& db_;
    const api::client::internal::Blockchain& blockchain_api_;
    const Type chain_;
    const SimpleCallback task_finished_;
    std::atomic_bool enabled_;
    OTZMQPushSocket thread_pool;
    wallet::Accounts accounts_;
    Proposals proposals_;

    auto pipeline(const zmq::Message& in) noexcept -> void;
    auto process_reorg(const zmq::Message& in) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;

    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::blockchain::client::implementation

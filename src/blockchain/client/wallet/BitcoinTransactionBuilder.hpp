// Copyright (c) 2010-2021 The Open-Transactions developers
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
#include <set>
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
#include "blockchain/client/wallet/Wallet.hpp"
#include "core/Worker.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
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
class Blockchain;
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Transaction;
}  // namespace internal
}  // namespace bitcoin
}  // namespace block

namespace client
{
namespace internal
{
struct WalletDatabase;
}  // namespace internal
}  // namespace client
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Identifier;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::blockchain::client::wallet
{
class BitcoinTransactionBuilder
{
public:
    using UTXO = std::
        pair<blockchain::block::Outpoint, proto::BlockchainTransactionOutput>;
    using Transaction = std::unique_ptr<block::bitcoin::internal::Transaction>;
    using KeyID = api::client::blockchain::Key;
    using Proposal = proto::BlockchainTransactionProposal;

    auto IsFunded() const noexcept -> bool;
    auto Spender() const noexcept -> const identifier::Nym&;

    auto AddChange(const Proposal& proposal) noexcept -> bool;
    auto AddInput(const UTXO& utxo) noexcept -> bool;
    auto CreateOutputs(const Proposal& proposal) noexcept -> bool;
    auto FinalizeOutputs() noexcept -> void;
    auto FinalizeTransaction() noexcept -> Transaction;
    auto ReleaseKeys() noexcept -> void;
    auto SignInputs() noexcept -> bool;

    BitcoinTransactionBuilder(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const internal::WalletDatabase& db,
        const Identifier& id,
        const Proposal& proposal,
        const Type chain,
        const Amount feeRate) noexcept;
    ~BitcoinTransactionBuilder();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    BitcoinTransactionBuilder() = delete;
};
}  // namespace opentxs::blockchain::client::wallet

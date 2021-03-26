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
namespace client
{
namespace internal
{
struct Network;
struct WalletDatabase;
}  // namespace internal
}  // namespace client
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::client::wallet
{
class Proposals
{
public:
    using Proposal = proto::BlockchainTransactionProposal;

    auto Add(const Proposal& tx) const noexcept -> std::future<block::pTxid>;

    auto Run() noexcept -> bool;

    Proposals(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const internal::Network& network,
        const internal::WalletDatabase& db,
        const Type chain) noexcept;
    ~Proposals();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::client::wallet

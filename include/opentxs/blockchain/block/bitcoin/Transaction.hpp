// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Contacts;
}  // namespace client

namespace crypto
{
class Blockchain;
}  // namespace crypto
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

class Inputs;
class Outputs;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace proto
{
class BlockchainTransaction;
class BlockchainTransactionOutput;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class OPENTXS_EXPORT Transaction
{
public:
    virtual auto AssociatedLocalNyms(const api::crypto::Blockchain& blockchain)
        const noexcept -> std::vector<OTNymID> = 0;
    virtual auto AssociatedRemoteContacts(
        const api::crypto::Blockchain& blockchain,
        const api::client::Contacts& contacts,
        const identifier::Nym& nym) const noexcept
        -> std::vector<OTIdentifier> = 0;
    virtual auto BlockPosition() const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Chains() const noexcept -> std::vector<blockchain::Type> = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Transaction> = 0;
    virtual auto ID() const noexcept -> const Txid& = 0;
    virtual auto Inputs() const noexcept -> const bitcoin::Inputs& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Transaction& = 0;
    virtual auto IsGeneration() const noexcept -> bool = 0;
    virtual auto Keys() const noexcept -> std::vector<crypto::Key> = 0;
    virtual auto Locktime() const noexcept -> std::uint32_t = 0;
    virtual auto Memo(const api::crypto::Blockchain& blockchain) const noexcept
        -> std::string = 0;
    virtual auto NetBalanceChange(
        const api::crypto::Blockchain& blockchain,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount = 0;
    virtual auto Outputs() const noexcept -> const bitcoin::Outputs& = 0;
    virtual auto Print() const noexcept -> std::string = 0;
    virtual auto SegwitFlag() const noexcept -> std::byte = 0;
    virtual auto Timestamp() const noexcept -> Time = 0;
    virtual auto Version() const noexcept -> std::int32_t = 0;
    virtual auto WTXID() const noexcept -> const Txid& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Transaction& = 0;

    virtual ~Transaction() = default;

protected:
    Transaction() noexcept = default;

private:
    Transaction(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    auto operator=(const Transaction&) -> Transaction& = delete;
    auto operator=(Transaction&&) -> Transaction& = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs

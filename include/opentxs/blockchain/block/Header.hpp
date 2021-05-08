// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_HEADER_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_HEADER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace proto
{
class BlockchainBlockHeader;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace block
{
class OPENTXS_EXPORT Header
{
public:
    using SerializedType = proto::BlockchainBlockHeader;

    enum class Status : std::uint32_t {
        Error,
        Normal,
        Disconnected,
        CheckpointBanned,
        Checkpoint
    };

    virtual std::unique_ptr<bitcoin::Header> as_Bitcoin() const noexcept = 0;
    virtual std::unique_ptr<Header> clone() const noexcept = 0;
    virtual OTWork Difficulty() const noexcept = 0;
    virtual Status EffectiveState() const noexcept = 0;
    virtual const block::Hash& Hash() const noexcept = 0;
    virtual block::Height Height() const noexcept = 0;
    virtual OTWork IncrementalWork() const noexcept = 0;
    virtual Status InheritedState() const noexcept = 0;
    virtual bool IsBlacklisted() const noexcept = 0;
    virtual bool IsDisconnected() const noexcept = 0;
    virtual Status LocalState() const noexcept = 0;
    virtual OTNumericHash NumericHash() const noexcept = 0;
    virtual const block::Hash& ParentHash() const noexcept = 0;
    virtual OTWork ParentWork() const noexcept = 0;
    virtual block::Position Position() const noexcept = 0;
    virtual std::string Print() const noexcept = 0;
    OPENTXS_NO_EXPORT virtual bool Serialize(
        SerializedType& out) const noexcept = 0;
    virtual auto Serialize(
        const AllocateOutput destination,
        const bool bitcoinformat = true) const noexcept -> bool = 0;
    virtual OTNumericHash Target() const noexcept = 0;
    virtual blockchain::Type Type() const noexcept = 0;
    virtual bool Valid() const noexcept = 0;
    virtual OTWork Work() const noexcept = 0;

    virtual void CompareToCheckpoint(
        const block::Position& checkpoint) noexcept = 0;
    /// Throws std::runtime_error if parent hash incorrect
    virtual void InheritHeight(const Header& parent) = 0;
    /// Throws std::runtime_error if parent hash incorrect
    virtual void InheritState(const Header& parent) = 0;
    virtual void InheritWork(const blockchain::Work& parent) noexcept = 0;
    virtual void RemoveBlacklistState() noexcept = 0;
    virtual void RemoveCheckpointState() noexcept = 0;
    virtual void SetDisconnectedState() noexcept = 0;

    virtual ~Header() = default;

protected:
    Header() noexcept = default;

private:
    Header(const Header&) = delete;
    Header(Header&&) = delete;
    Header& operator=(const Header&) = delete;
    Header& operator=(Header&&) = delete;
};
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif

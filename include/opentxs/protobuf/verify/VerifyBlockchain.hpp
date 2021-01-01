// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFYBLOCKCHAIN_HPP
#define OPENTXS_PROTOBUF_VERIFYBLOCKCHAIN_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT const VersionMap& Bip47ChainAllowedBip47Channel() noexcept;
OPENTXS_EXPORT const VersionMap& Bip47ChannelAllowedBip47Direction() noexcept;
OPENTXS_EXPORT const VersionMap& Bip47ContextAllowedBip47Chain() noexcept;
OPENTXS_EXPORT const VersionMap& Bip47DirectionAllowedBip47Address() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainAddressAllowedAsymmetricKey() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainBlockHeaderAllowedBitcoinBlockHeaderFields() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainBlockHeaderAllowedBlockchainBlockLocalData() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainBlockHeaderAllowedEthereumBlockHeaderFields() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainP2PHelloAllowedBlockchainP2PChainState() noexcept;
OPENTXS_EXPORT const VersionMap& BlockchainTransactionAllowedInput() noexcept;
OPENTXS_EXPORT const VersionMap& BlockchainTransactionAllowedOutput() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainTransactionInputAllowedBlockchainInputWitness() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainTransactionInputAllowedBlockchainPreviousOutput() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainTransactionInputAllowedBlockchainTransactionOutput() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainTransactionInputAllowedBlockchainWalletKey() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainTransactionOutputAllowedBlockchainWalletKey() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainTransactionProposalAllowedBlockchainTransaction() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainTransactionProposalAllowedBlockchainTransactionProposedOutput() noexcept;
OPENTXS_EXPORT const VersionMap&
BlockchainTransactionProposedOutputAllowedBlockchainOutputMultisigDetails() noexcept;
OPENTXS_EXPORT const VersionMap& HDAccountAllowedBlockchainActivity() noexcept;
OPENTXS_EXPORT const VersionMap& HDAccountAllowedBlockchainAddress() noexcept;
OPENTXS_EXPORT const VersionMap& HDAccountAllowedHDPath() noexcept;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYBLOCKCHAIN_HPP

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(feeoracle)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/Accounts.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/FeeOracle.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/FeeSource.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/Types.hpp"
    "Account.cpp"
    "Account.hpp"
    "Accounts.cpp"
    "Accounts.hpp"
    "Actor.hpp"
    "Batch.cpp"
    "Batch.hpp"
    "BitcoinTransactionBuilder.cpp"
    "BitcoinTransactionBuilder.hpp"
    "BlockIndex.cpp"
    "BlockIndex.hpp"
    "DeterministicIndex.cpp"
    "DeterministicStateData.cpp"
    "DeterministicStateData.hpp"
    "Index.cpp"
    "Index.hpp"
    "Job.cpp"
    "Job.hpp"
    "Mempool.cpp"
    "Mempool.hpp"
    "NotificationIndex.cpp"
    "NotificationStateData.cpp"
    "NotificationStateData.hpp"
    "Process.cpp"
    "Process.hpp"
    "Progress.cpp"
    "Progress.hpp"
    "Proposals.cpp"
    "Proposals.hpp"
    "Rescan.cpp"
    "Rescan.hpp"
    "Scan.cpp"
    "Scan.hpp"
    "ScriptForm.cpp"
    "ScriptForm.hpp"
    "SubchainStateData.cpp"
    "SubchainStateData.hpp"
    "Types.hpp"
    "Wallet.cpp"
    "Wallet.hpp"
    "Work.cpp"
    "Work.hpp"
)
target_link_libraries(opentxs-common PRIVATE Boost::headers)

# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-blockchain-node-wallet OBJECT
  "Account.cpp"
  "Account.hpp"
  "Accounts.cpp"
  "Accounts.hpp"
  "BitcoinTransactionBuilder.cpp"
  "BitcoinTransactionBuilder.hpp"
  "DeterministicStateData.cpp"
  "DeterministicStateData.hpp"
  "NotificationStateData.cpp"
  "NotificationStateData.hpp"
  "Proposals.cpp"
  "Proposals.hpp"
  "ScriptForm.cpp"
  "ScriptForm.hpp"
  "SubchainStateData.cpp"
  "SubchainStateData.hpp"
  "Wallet.cpp"
  "Wallet.hpp"
)
target_link_libraries(
  opentxs-blockchain-node-wallet PRIVATE Boost::headers opentxs::messages
)
target_include_directories(
  opentxs-blockchain-node-wallet SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-blockchain-node-wallet PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-node-wallet>)
add_dependencies(opentxs-blockchain-node-wallet generated_code)

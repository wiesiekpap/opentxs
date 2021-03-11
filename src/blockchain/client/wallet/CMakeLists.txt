# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-blockchain-client-wallet OBJECT
  "Account.cpp"
  "Account.hpp"
  "Accounts.cpp"
  "Accounts.hpp"
  "BitcoinTransactionBuilder.cpp"
  "DeterministicStateData.cpp"
  "DeterministicStateData.hpp"
  "NotificationStateData.cpp"
  "NotificationStateData.hpp"
  "Proposals.cpp"
  "SubchainStateData.cpp"
  "SubchainStateData.hpp"
  "Wallet.cpp"
  "Wallet.hpp"
)
target_link_libraries(
  opentxs-blockchain-client-wallet PRIVATE Boost::headers opentxs::messages
)
target_include_directories(
  opentxs-blockchain-client-wallet SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-blockchain-client-wallet PRIVATE ${cxx-install-headers})
target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-client-wallet>
)
add_dependencies(opentxs-blockchain-client-wallet generated_code)

# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    Account.cpp
    Accounts.cpp
    BitcoinTransactionBuilder.cpp
    DeterministicStateData
    NotificationStateData.cpp
    Proposals.cpp
    SubchainStateData.cpp
    Wallet.cpp
)

set(cxx-install-headers "")

set(cxx-headers
    ${cxx-install-headers}
    Account.hpp
    Accounts.hpp
    DeterministicStateData
    NotificationStateData.hpp
    SubchainStateData.hpp
    Wallet.hpp
)

add_library(
  opentxs-blockchain-client-wallet OBJECT ${cxx-sources} ${cxx-headers}
)
target_link_libraries(
  opentxs-blockchain-client-wallet PRIVATE Boost::headers opentxs::messages
)
target_include_directories(
  opentxs-blockchain-client-wallet SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
add_dependencies(opentxs-blockchain-client-wallet generated_code)

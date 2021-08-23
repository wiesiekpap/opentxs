# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-ui-accountactivity OBJECT
  "AccountActivity.cpp"
  "AccountActivity.hpp"
  "BalanceItem.cpp"
  "BalanceItem.hpp"
  "ChequeBalanceItem.cpp"
  "ChequeBalanceItem.hpp"
  "CustodialAccountActivity.cpp"
  "CustodialAccountActivity.hpp"
  "TransferBalanceItem.cpp"
  "TransferBalanceItem.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountActivity.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/BalanceItem.hpp"
)
target_link_libraries(opentxs-ui-accountactivity PRIVATE opentxs::messages)
target_include_directories(
  opentxs-ui-accountactivity SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-ui-accountactivity
    PRIVATE
      "BlockchainAccountActivity.cpp"
      "BlockchainAccountActivity.hpp"
      "BlockchainBalanceItem.cpp"
      "BlockchainBalanceItem.hpp"
  )
  target_link_libraries(opentxs-ui-accountactivity PRIVATE Boost::headers)
endif()

target_sources(opentxs-ui-accountactivity PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-accountactivity>)
add_dependencies(opentxs-ui-accountactivity generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()

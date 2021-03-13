# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(database)
endif()

add_library(
  opentxs-api-client-blockchain OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/api/client/blockchain/Blockchain.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/api/client/blockchain/Factory.hpp"
  "AccountCache.cpp"
  "BalanceList.cpp"
  "BalanceList.hpp"
  "BalanceLists.cpp"
  "BalanceNode.cpp"
  "BalanceNode.hpp"
  "BalanceTree.cpp"
  "BalanceTree.hpp"
  "BalanceTree.tpp"
  "Blockchain.cpp"
  "Deterministic.cpp"
  "Deterministic.hpp"
  "Element.cpp"
  "HD.cpp"
  "HD.hpp"
  "PaymentCode.cpp"
  "PaymentCode.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/AddressStyle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/BalanceList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/BalanceNode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/BalanceNodeType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/BalanceTree.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/Deterministic.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/Ethereum.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/HD.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/Imported.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/PaymentCode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/Subchain.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/Types.hpp"
)
target_link_libraries(
  opentxs-api-client-blockchain PRIVATE opentxs::messages Boost::headers
)
target_include_directories(
  opentxs-api-client-blockchain SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-api-client-blockchain
    PRIVATE
      "BalanceOracle.cpp"
      "SyncClient.cpp"
      "SyncClient.hpp"
      "SyncServer.cpp"
      "SyncServer.hpp"
  )
  target_link_libraries(
    opentxs-api-client-blockchain PRIVATE "${OT_ZMQ_TARGET}"
  )
  target_link_libraries(opentxs PUBLIC "${OT_ZMQ_TARGET}")
endif()

target_sources(opentxs-api-client-blockchain PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-client-blockchain>)
add_dependencies(opentxs-api-client-blockchain generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/client/blockchain
    COMPONENT dev
  )
endif()

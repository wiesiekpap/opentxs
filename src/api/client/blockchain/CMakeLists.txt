# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(database)
endif()

set(cxx-sources
    AccountCache.cpp
    BalanceList.cpp
    BalanceLists.cpp
    BalanceNode.cpp
    BalanceTree.cpp
    BalanceTree.tpp
    Blockchain.cpp
    Deterministic.cpp
    HD.cpp
)

if(OT_BLOCKCHAIN_EXPORT)
  list(
    APPEND
    cxx-sources
    BalanceOracle.cpp
    EnableCallbacks.cpp
    SyncServer.cpp
    ThreadPoolManager.cpp
  )
endif()

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
    "${opentxs_SOURCE_DIR}/src/internal/api/client/blockchain/Blockchain.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/client/blockchain/Factory.hpp"
    ${cxx-install-headers}
)
set(cxx-headers
    BalanceList.hpp
    BalanceNode.hpp
    BalanceTree.hpp
    Deterministic.hpp
    HD.hpp
)

add_library(opentxs-api-client-blockchain OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-api-client-blockchain PRIVATE opentxs::messages)

if(OT_BLOCKCHAIN_EXPORT)
  target_link_libraries(
    opentxs-api-client-blockchain PRIVATE "${OT_ZMQ_TARGET}" Boost::headers
  )
endif()

target_include_directories(
  opentxs-api-client-blockchain SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
add_dependencies(opentxs-api-client-blockchain generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/client/blockchain
    COMPONENT dev
  )
endif()

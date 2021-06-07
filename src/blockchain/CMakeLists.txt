# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(crypto)

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(bitcoin)
  add_subdirectory(block)
  add_subdirectory(database)
  add_subdirectory(node)
  add_subdirectory(p2p)
endif()

add_library(
  opentxs-blockchain OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/Params.hpp"
  "DownloadManager.hpp"
  "DownloadTask.hpp"
  "NumericHash.cpp"
  "NumericHash.hpp"
  "Params.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Blockchain.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/BlockchainType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/BloomUpdateFlag.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/FilterType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/SendResult.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Types.hpp"
)
target_link_libraries(
  opentxs-blockchain PRIVATE Boost::headers opentxs::messages
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-blockchain
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/Blockchain.hpp"
      "Blockchain.cpp"
      "BloomFilter.cpp"
      "GCS.cpp"
      "GCS.hpp"
      "Work.cpp"
      "Work.hpp"
  )
  list(
    APPEND
    cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/BloomFilter.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/NumericHash.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Work.hpp"
  )
  target_link_libraries(opentxs-blockchain PRIVATE lmdb)
  target_link_libraries(opentxs PUBLIC lmdb)
endif()

target_sources(opentxs-blockchain PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain>)
add_dependencies(opentxs-blockchain generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain
    COMPONENT dev
  )
endif()

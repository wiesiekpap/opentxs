# Copyright (c) 2010-2022 The Open-Transactions developers
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

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/Params.hpp"
    "DownloadManager.hpp"
    "DownloadTask.hpp"
    "NumericHash.cpp"
    "NumericHash.hpp"
    "Params.cpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Blockchain.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/BlockchainType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/BloomUpdateFlag.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/FilterType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/GCS.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/SendResult.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Types.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
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
  target_link_libraries(opentxs-common PRIVATE lmdb)
  target_link_libraries(opentxs PUBLIC lmdb)
else()
  # TODO move to appropriate subdirectories
  list(
    APPEND
    cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Types.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/Types.hpp"
  )
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain
    COMPONENT dev
  )
endif()

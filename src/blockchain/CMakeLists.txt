# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(bitcoin)
  add_subdirectory(block)
  add_subdirectory(client)
  add_subdirectory(database)
  add_subdirectory(p2p)
endif()

set(cxx-sources Params.cpp)

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Blockchain.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/BlockchainType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/BloomUpdateFlag.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/FilterType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Types.hpp"
)

set(cxx-headers
    ${cxx-install-headers}
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/Params.hpp"
    "DownloadManager.hpp"
    "DownloadTask.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  list(
    APPEND
    cxx-sources
    Blockchain.cpp
    BloomFilter.cpp
    GCS.cpp
    NumericHash.cpp
    Work.cpp
  )

  list(
    APPEND
    cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/BloomFilter.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Network.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/NumericHash.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Work.hpp"
  )

  list(
    APPEND
    cxx-headers
    ${cxx-install-headers}
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/Blockchain.hpp"
    GCS.hpp
    NumericHash.hpp
    Work.hpp
  )
endif()

add_library(opentxs-blockchain OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(
  opentxs-blockchain PRIVATE Boost::headers opentxs::messages
)

add_dependencies(opentxs-blockchain generated_code)

if(OT_BLOCKCHAIN_EXPORT)
  target_link_libraries(opentxs-blockchain PRIVATE lmdb)
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain
    COMPONENT dev
  )
endif()

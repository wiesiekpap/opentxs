# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(base)
add_subdirectory(bitcoin)
add_subdirectory(wallet)

add_library(
  opentxs-blockchain-node OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Node.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Factory.hpp"
  "blockoracle/Cache.cpp"
  "blockoracle/Mem.cpp"
  "filteroracle/BlockIndexer.cpp"
  "filteroracle/BlockIndexer.hpp"
  "filteroracle/FilterCheckpoints.hpp"
  "filteroracle/FilterDownloader.hpp"
  "filteroracle/HeaderDownloader.hpp"
  "peermanager/IncomingConnectionManager.hpp"
  "peermanager/Jobs.cpp"
  "peermanager/Peers.cpp"
  "peermanager/ZMQ.cpp"
  "BlockOracle.cpp"
  "BlockOracle.hpp"
  "FilterOracle.cpp"
  "FilterOracle.hpp"
  "HeaderOracle.cpp"
  "HeaderOracle.hpp"
  "Node.cpp"
  "PeerManager.cpp"
  "PeerManager.hpp"
  "UpdateTransaction.cpp"
  "UpdateTransaction.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/BlockOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/FilterOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/HeaderOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/Manager.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/Wallet.hpp"
)

target_link_libraries(
  opentxs-blockchain-node
  PRIVATE
    Boost::headers
    opentxs::messages
    "${OT_ZMQ_TARGET}"
)
target_link_libraries(opentxs PUBLIC "${OT_ZMQ_TARGET}")
target_include_directories(
  opentxs-blockchain-node SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(PACKETCRYPT_EXPORT)
  target_sources(
    opentxs-blockchain-node PRIVATE "blockoracle/GetValidator_packetcrypt.cpp"
  )
else()
  target_sources(opentxs-blockchain-node PRIVATE "blockoracle/GetValidator.cpp")
endif()

target_sources(opentxs-blockchain-node PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-node>)
add_dependencies(opentxs-blockchain-node generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/node
    COMPONENT dev
  )
endif()

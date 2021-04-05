# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bitcoin)
add_subdirectory(wallet)

add_library(
  opentxs-blockchain-client OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/client/Client.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/client/Factory.hpp"
  "BlockOracle.cpp"
  "BlockOracle.hpp"
  "Client.cpp"
  "FilterOracle.cpp"
  "FilterOracle.hpp"
  "HeaderOracle.cpp"
  "HeaderOracle.hpp"
  "Network.cpp"
  "Network.hpp"
  "PeerManager.cpp"
  "PeerManager.hpp"
  "UpdateTransaction.cpp"
  "UpdateTransaction.hpp"
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
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/client/BlockOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/client/FilterOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/client/HeaderOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/client/Wallet.hpp"
)

target_link_libraries(
  opentxs-blockchain-client
  PRIVATE
    Boost::headers
    opentxs::messages
    "${OT_ZMQ_TARGET}"
)
target_link_libraries(opentxs PUBLIC "${OT_ZMQ_TARGET}")
target_include_directories(
  opentxs-blockchain-client SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(PACKETCRYPT_EXPORT)
  target_sources(
    opentxs-blockchain-client
    PRIVATE "blockoracle/GetValidator_packetcrypt.cpp"
  )
else()
  target_sources(
    opentxs-blockchain-client PRIVATE "blockoracle/GetValidator.cpp"
  )
endif()

target_sources(opentxs-blockchain-client PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-client>)
add_dependencies(opentxs-blockchain-client generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/client
    COMPONENT dev
  )
endif()

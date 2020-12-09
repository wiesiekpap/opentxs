# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bitcoin)

set(cxx-sources
    blockoracle/Cache.cpp
    blockoracle/Mem.cpp
    peermanager/Jobs.cpp
    peermanager/Peers.cpp
    peermanager/ZMQ.cpp
    wallet/Account.cpp
    wallet/Accounts.cpp
    wallet/BitcoinTransactionBuilder.cpp
    wallet/HDStateData.cpp
    wallet/Proposals.cpp
    BlockOracle.cpp
    Client.cpp
    FilterOracle.cpp
    HeaderOracle.cpp
    Network.cpp
    PeerManager.cpp
    UpdateTransaction.cpp
    Wallet.cpp
)

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/client/BlockOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/client/FilterOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/client/HeaderOracle.hpp"
)

set(cxx-headers
    ${cxx-install-headers}
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/client/Client.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/client/Factory.hpp"
    filteroracle/BlockIndexer.hpp
    filteroracle/FilterCheckpoints.hpp
    filteroracle/FilterDownloader.hpp
    filteroracle/HeaderDownloader.hpp
    peermanager/IncomingConnectionManager.hpp
    wallet/HDStateData.hpp
    BlockOracle.hpp
    FilterOracle.hpp
    HeaderOracle.hpp
    Network.hpp
    PeerManager.hpp
    UpdateTransaction.hpp
    Wallet.hpp
)

add_library(opentxs-blockchain-client OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(
  opentxs-blockchain-client PRIVATE Boost::headers opentxs::messages
                                    "${OT_ZMQ_TARGET}"
)
target_include_directories(
  opentxs-blockchain-client SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
add_dependencies(opentxs-blockchain-client generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/client
    COMPONENT dev
  )
endif()

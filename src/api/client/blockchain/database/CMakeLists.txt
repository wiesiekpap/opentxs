# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    BlockFilter.cpp
    BlockHeaders.cpp
    Database.cpp
    Peers.cpp
    Wallet.cpp
)
set(cxx-headers
    BlockFilter.hpp
    BlockHeaders.hpp
    Database.hpp
    Peers.hpp
    Wallet.hpp
)

if(OPENTXS_BLOCK_STORAGE_ENABLED)
  list(
    APPEND
    cxx-sources
    Blocks.cpp
    Sync.cpp
  )
  list(
    APPEND
    cxx-headers
    Blocks.hpp
    Sync.hpp
  )
endif()

add_library(
  opentxs-api-client-blockchain-database OBJECT ${cxx-sources} ${cxx-headers}
)
target_link_libraries(
  opentxs-api-client-blockchain-database
  PRIVATE
    opentxs::messages
    unofficial-sodium::sodium
    Boost::headers
    lmdb
)

if(OPENTXS_BLOCK_STORAGE_ENABLED)
  target_compile_definitions(
    opentxs-api-client-blockchain-database
    PRIVATE OPENTXS_BLOCK_STORAGE_ENABLED=1
  )
else()
  target_compile_definitions(
    opentxs-api-client-blockchain-database
    PRIVATE OPENTXS_BLOCK_STORAGE_ENABLED=0
  )
endif()

target_compile_definitions(
  opentxs-api-client-blockchain-database
  PRIVATE
    OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY=${OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY}
)
add_dependencies(opentxs-api-client-blockchain-database generated_code)

# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-api-client-blockchain-database OBJECT
  "BlockFilter.cpp"
  "BlockFilter.hpp"
  "BlockHeaders.cpp"
  "BlockHeaders.hpp"
  "Config.cpp"
  "Config.hpp"
  "Database.cpp"
  "Database.hpp"
  "Peers.cpp"
  "Peers.hpp"
  "Wallet.cpp"
  "Wallet.hpp"
)

if(OPENTXS_BLOCK_STORAGE_ENABLED)
  target_sources(
    opentxs-api-client-blockchain-database
    PRIVATE
      "Blocks.cpp"
      "Blocks.hpp"
      "Sync.cpp"
      "Sync.hpp"
  )
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

target_link_libraries(
  opentxs-api-client-blockchain-database
  PRIVATE
    opentxs::messages
    unofficial-sodium::sodium
    Boost::headers
    lmdb
)
target_link_libraries(
  opentxs
  PUBLIC
    unofficial-sodium::sodium
    Boost::thread
    lmdb
)
target_compile_definitions(
  opentxs-api-client-blockchain-database
  PRIVATE
    OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY=${OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY}
)

if(WIN32 AND OT_STATIC_DEPENDENCIES)
  target_compile_definitions(
    opentxs-api-client-blockchain-database PRIVATE SODIUM_STATIC=1
                                                   SODIUM_EXPORT=
  )
endif()

target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-client-blockchain-database>
)
add_dependencies(opentxs-api-client-blockchain-database generated_code)

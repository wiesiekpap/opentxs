# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-blockchain-database-common OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/common/Common.hpp"
  "BlockFilter.cpp"
  "BlockFilter.hpp"
  "BlockHeaders.cpp"
  "BlockHeaders.hpp"
  "Bulk.cpp"
  "Bulk.hpp"
  "Common.cpp"
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
    opentxs-blockchain-database-common
    PRIVATE
      "Blocks.cpp"
      "Blocks.hpp"
      "Sync.cpp"
      "Sync.hpp"
  )
  target_compile_definitions(
    opentxs-blockchain-database-common PRIVATE OPENTXS_BLOCK_STORAGE_ENABLED=1
  )
else()
  target_compile_definitions(
    opentxs-blockchain-database-common PRIVATE OPENTXS_BLOCK_STORAGE_ENABLED=0
  )
endif()

target_link_libraries(
  opentxs-blockchain-database-common
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
    Boost::filesystem
    lmdb
)
target_compile_definitions(
  opentxs-blockchain-database-common
  PRIVATE
    OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY=${OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY}
)

if(WIN32 AND OT_STATIC_DEPENDENCIES)
  target_compile_definitions(
    opentxs-blockchain-database-common PRIVATE SODIUM_STATIC=1 SODIUM_EXPORT=
  )
endif()

target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-database-common>
)
add_dependencies(opentxs-blockchain-database-common generated_code)

# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-blockchain-database OBJECT "")

add_subdirectory(common)
add_subdirectory(wallet)

target_sources(
  opentxs-blockchain-database
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Database.hpp"
    "Blocks.cpp"
    "Blocks.hpp"
    "Database.cpp"
    "Database.hpp"
    "Filters.cpp"
    "Filters.hpp"
    "Headers.cpp"
    "Headers.hpp"
    "Sync.cpp"
    "Sync.hpp"
    "Wallet.cpp"
    "Wallet.hpp"
)
target_link_libraries(
  opentxs-blockchain-database
  PRIVATE
    Boost::headers
    opentxs::messages
    lmdb
)
target_link_libraries(opentxs PUBLIC lmdb)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-database>)
add_dependencies(opentxs-blockchain-database generated_code)

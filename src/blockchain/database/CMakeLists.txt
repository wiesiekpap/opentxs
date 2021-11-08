# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(common)
add_subdirectory(wallet)

target_sources(
  opentxs-common
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
target_link_libraries(opentxs-common PRIVATE lmdb)
target_link_libraries(opentxs PUBLIC lmdb)

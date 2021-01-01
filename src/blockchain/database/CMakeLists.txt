# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    Blocks.cpp
    Database.cpp
    Filters.cpp
    Headers.cpp
    Sync.cpp
    Wallet.cpp
)

set(cxx-headers
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Database.hpp"
    Blocks.hpp
    Database.hpp
    Filters.hpp
    Headers.hpp
    Sync.hpp
    Wallet.hpp
)

add_library(opentxs-blockchain-database OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(
  opentxs-blockchain-database PRIVATE Boost::headers opentxs::messages lmdb
)
add_dependencies(opentxs-blockchain-database generated_code)

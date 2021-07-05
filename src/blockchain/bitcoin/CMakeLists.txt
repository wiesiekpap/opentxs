# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-blockchain-bitcoin OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/Bitcoin.hpp"
  "Bitcoin.cpp"
  "Inventory.cpp"
  "Inventory.hpp"
)
target_link_libraries(
  opentxs-blockchain-bitcoin PRIVATE Boost::headers opentxs::messages
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-bitcoin>)
add_dependencies(opentxs-blockchain-bitcoin generated_code)

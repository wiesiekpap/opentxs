# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(message)

add_library(
  opentxs-blockchain-p2p-bitcoin OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/p2p/bitcoin/Factory.hpp"
  "Bitcoin.cpp"
  "Header.cpp"
  "Header.hpp"
  "Message.cpp"
  "Message.hpp"
  "Peer.cpp"
  "Peer.hpp"
)
target_link_libraries(
  opentxs-blockchain-p2p-bitcoin PRIVATE Boost::headers opentxs::messages
)
target_include_directories(
  opentxs-blockchain-p2p-bitcoin SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-p2p-bitcoin>)
add_dependencies(opentxs-blockchain-p2p-bitcoin generated_code)

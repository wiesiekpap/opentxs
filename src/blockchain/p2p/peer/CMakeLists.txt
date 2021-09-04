# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-blockchain-p2p-peer OBJECT
  "Activity.cpp"
  "Address.cpp"
  "DownloadPeers.cpp"
  "SendPromises.cpp"
  "TCP.cpp"
  "ZMQ.cpp"
  "Peer.cpp"
  "Peer.hpp"
)
target_link_libraries(
  opentxs-blockchain-p2p-peer PRIVATE Boost::headers opentxs::messages
)
target_include_directories(
  opentxs-blockchain-p2p-peer SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
target_link_libraries(opentxs PUBLIC Boost::system)
target_sources(opentxs-blockchain-p2p-peer PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-p2p-peer>)
add_dependencies(opentxs-blockchain-p2p-peer generated_code)

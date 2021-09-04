# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-blockchain-node-peermanager OBJECT
  "IncomingConnectionManager.hpp"
  "Jobs.cpp"
  "PeerManager.cpp"
  "PeerManager.hpp"
  "Peers.cpp"
  "TCP.cpp"
  "ZMQ.cpp"
)

target_link_libraries(
  opentxs-blockchain-node-peermanager PRIVATE Boost::headers opentxs::messages
)
target_link_libraries(opentxs PUBLIC Boost::system)
target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-node-peermanager>
)
add_dependencies(opentxs-blockchain-node-peermanager generated_code)

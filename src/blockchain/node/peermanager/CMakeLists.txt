# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/PeerManager.hpp"
    "IncomingConnectionManager.hpp"
    "Jobs.cpp"
    "PeerManager.cpp"
    "PeerManager.hpp"
    "Peers.cpp"
    "TCP.cpp"
    "ZMQ.cpp"
)
target_link_libraries(opentxs PUBLIC Boost::system)

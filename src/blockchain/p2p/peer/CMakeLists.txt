# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "Activity.cpp"
    "Address.cpp"
    "Address.hpp"
    "ConnectionManager.hpp"
    "DownloadPeers.cpp"
    "Peer.cpp"
    "Peer.hpp"
    "SendPromises.cpp"
    "TCP.cpp"
    "ZMQ.cpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
target_link_libraries(opentxs PUBLIC Boost::system)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

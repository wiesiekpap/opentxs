# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-api-network-blockchain OBJECT "")

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-api-network-blockchain
    PRIVATE
      "Factory.cpp"
      "Imp.cpp"
      "Imp.hpp"
      "SyncClient.cpp"
      "SyncClient.hpp"
      "SyncServer.cpp"
      "SyncServer.hpp"
  )
else()
  target_sources(opentxs-api-network-blockchain PRIVATE "Null.cpp")
endif()

target_link_libraries(
  opentxs-api-network-blockchain
  PRIVATE
    opentxs::messages
    "${OT_ZMQ_TARGET}"
    Boost::headers
)
target_link_libraries(
  opentxs
  PUBLIC
    "${OT_ZMQ_TARGET}"
    Boost::system
    Boost::thread
)
target_include_directories(
  opentxs-api-network-blockchain SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-network-blockchain>)
add_dependencies(opentxs-api-network-blockchain generated_code)

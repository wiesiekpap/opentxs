# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-network-zeromq-curve OBJECT
  "Client.cpp"
  "Client.hpp"
  "Server.cpp"
  "Server.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/curve/Client.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/curve/Server.hpp"
)
target_link_libraries(
  opentxs-network-zeromq-curve PRIVATE opentxs::messages "${OT_ZMQ_TARGET}"
)
target_link_libraries(opentxs PUBLIC "${OT_ZMQ_TARGET}")
target_sources(opentxs-network-zeromq-curve PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-network-zeromq-curve>)
add_dependencies(opentxs-network-zeromq-curve generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/zeromq/curve
    COMPONENT dev
  )
endif()

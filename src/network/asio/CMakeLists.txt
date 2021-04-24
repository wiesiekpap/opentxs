# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-network-asio OBJECT
  "Endpoint.cpp"
  "Endpoint.hpp"
  "Socket.cpp"
  "Socket.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/asio/Endpoint.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/asio/Socket.hpp"
)
target_link_libraries(
  opentxs-network-asio PRIVATE opentxs::messages Boost::headers
)
target_link_libraries(opentxs PUBLIC Boost::system)
target_sources(opentxs-network-asio PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-network-asio>)
add_dependencies(opentxs-network-asio generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/asio
    COMPONENT dev
  )
endif()

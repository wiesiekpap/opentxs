# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(asio)
add_subdirectory(blockchain)
add_subdirectory(zeromq)

add_library(
  opentxs-network OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/network/Factory.hpp"
  "ServerConnection.cpp"
  "ServerConnection.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/OpenDHT.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/ServerConnection.hpp"
)
target_link_libraries(opentxs-network PRIVATE opentxs::messages)
target_include_directories(
  opentxs-network SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(DHT_EXPORT)
  target_sources(opentxs-network PRIVATE "OpenDHT.cpp" "OpenDHT.hpp")
  target_link_libraries(opentxs-network PRIVATE opendht ${GNUTLS_LIBRARIES})
  target_link_libraries(opentxs PUBLIC opendht ${GNUTLS_LIBRARIES})
else()
  target_sources(opentxs-network PRIVATE "Null.cpp")
endif()

target_sources(opentxs-network PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-network>)
add_dependencies(opentxs-network generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network
    COMPONENT dev
  )
endif()

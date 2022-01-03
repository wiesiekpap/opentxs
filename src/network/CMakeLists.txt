# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(asio)
add_subdirectory(blockchain)
add_subdirectory(p2p)
add_subdirectory(zeromq)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/Factory.hpp"
    "ServerConnection.cpp"
    "ServerConnection.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/OpenDHT.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/ServerConnection.hpp"
)

if(DHT_EXPORT)
  target_sources(opentxs-common PRIVATE "OpenDHT.cpp" "OpenDHT.hpp")
  target_link_libraries(opentxs-common PRIVATE opendht ${GNUTLS_LIBRARIES})
  target_link_libraries(opentxs PUBLIC opendht ${GNUTLS_LIBRARIES})
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-api-server OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/api/server/Server.hpp"
  "Factory.cpp"
  "Factory.hpp"
  "Manager.cpp"
  "Manager.hpp"
  "Wallet.cpp"
  "Wallet.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/server/Manager.hpp"
)
target_link_libraries(opentxs-api-server PRIVATE opentxs::messages)
target_include_directories(
  opentxs-api-server SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

target_sources(opentxs-api-server PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-server>)
add_dependencies(opentxs-api-server generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/server
    COMPONENT dev
  )
endif()

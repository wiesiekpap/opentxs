# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-api-session-client OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/api/session/Client.hpp"
  "Client.cpp"
  "Client.hpp"
  "Factory.cpp"
  "Factory.hpp"
  "Wallet.cpp"
  "Wallet.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/session/Notary.hpp"
)
target_link_libraries(
  opentxs-api-session-client PRIVATE Boost::headers opentxs-common
)
target_include_directories(
  opentxs-api-session-client SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-api-session-client PRIVATE ${cxx-install-headers})

if(NOT OT_IWYU)
  target_precompile_headers(
    opentxs-api-session-client
    REUSE_FROM
    opentxs-common
  )
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-session-client>)
add_dependencies(opentxs-api-session-client generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/session/client
    COMPONENT dev
  )
endif()

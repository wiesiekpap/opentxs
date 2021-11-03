# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(client)
add_subdirectory(context)
add_subdirectory(crypto)
add_subdirectory(network)
add_subdirectory(session)

add_library(
  opentxs-api OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/api/Crypto.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/api/Factory.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/api/FactoryAPI.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/api/Legacy.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/api/Log.hpp"
  "Factory.cpp"
  "Factory.hpp"
  "Legacy.cpp"
  "Legacy.hpp"
  "Log.cpp"
  "Log.hpp"
  "Periodic.cpp"
  "Periodic.hpp"
  "Settings.cpp"
  "Settings.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Factory.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Periodic.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Settings.hpp"
)

target_link_libraries(opentxs-api PRIVATE Boost::headers opentxs-common)
target_link_libraries(opentxs PUBLIC Boost::filesystem)
target_include_directories(
  opentxs-api SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-api PRIVATE ${cxx-install-headers})

if(NOT OT_IWYU)
  target_precompile_headers(
    opentxs-api
    REUSE_FROM
    opentxs-common
  )
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api>)
add_dependencies(opentxs-api generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api
    COMPONENT dev
  )
endif()

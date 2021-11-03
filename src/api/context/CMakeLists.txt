# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-api-context OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/api/Context.hpp"
  "Context.cpp"
  "Context.hpp"
)
set(cxx-install-headers "${opentxs_SOURCE_DIR}/include/opentxs/api/Context.hpp")

if(OT_ENABLE_RPC)
  target_sources(opentxs-api-context PRIVATE "rpc.cpp")
else()
  target_sources(opentxs-api-context PRIVATE "no-rpc.cpp")
endif()

if(OT_QT_EXPORT)
  target_sources(opentxs-api-context PRIVATE "Qt.cpp")
  target_link_libraries(opentxs-api-context PRIVATE Qt5::Core)
  target_link_libraries(opentxs PRIVATE Qt5::Core)
else()
  target_sources(opentxs-api-context PRIVATE "no-Qt.cpp")
endif()

target_link_libraries(opentxs-api-context PRIVATE opentxs::messages)
target_include_directories(
  opentxs-api-context SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-api-context PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-context>)
add_dependencies(opentxs-api-context generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api
    COMPONENT dev
  )
endif()

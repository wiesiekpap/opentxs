# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(client)
add_subdirectory(context)
add_subdirectory(crypto)
add_subdirectory(network)
add_subdirectory(server)
add_subdirectory(storage)

add_library(
  opentxs-api OBJECT
  "${opentxs_SOURCE_DIR}/include/opentxs/api/Legacy.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/api/Api.hpp"
  "Context.cpp"
  "Context.hpp"
  "Core.cpp"
  "Core.hpp"
  "Endpoints.cpp"
  "Endpoints.hpp"
  "Factory.cpp"
  "Factory.hpp"
  "HDSeed.cpp"
  "HDSeed.hpp"
  "Legacy.cpp"
  "Legacy.hpp"
  "Log.cpp"
  "Log.hpp"
  "Options.cpp"
  "Options.hpp"
  "Periodic.cpp"
  "Periodic.hpp"
  "Primitives.cpp"
  "Primitives.hpp"
  "Scheduler.cpp"
  "Scheduler.hpp"
  "Settings.cpp"
  "Settings.hpp"
  "StorageParent.cpp"
  "StorageParent.hpp"
  "Wallet.cpp"
  "Wallet.hpp"
  "ZMQ.cpp"
  "ZMQ.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Context.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Core.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Editor.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Endpoints.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Factory.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/HDSeed.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Options.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Periodic.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Primitives.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Settings.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Wallet.hpp"
)

if(OT_QT_EXPORT)
  target_sources(opentxs-api PRIVATE "Qt.cpp")
  target_link_libraries(opentxs-api PRIVATE Qt5::Core)
  target_link_libraries(opentxs PRIVATE Qt5::Core)
else()
  target_sources(opentxs-api PRIVATE "no-Qt.cpp")
endif()

target_link_libraries(opentxs-api PRIVATE Boost::headers opentxs::messages)
target_link_libraries(opentxs PUBLIC Boost::filesystem Boost::program_options)
target_include_directories(
  opentxs-api SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-api PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api>)
add_dependencies(opentxs-api generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(client)
add_subdirectory(crypto)
add_subdirectory(network)
add_subdirectory(server)
add_subdirectory(storage)

set(cxx-sources
    Context.cpp
    Core.cpp
    Endpoints.cpp
    Factory.cpp
    HDSeed.cpp
    Legacy.cpp
    Log.cpp
    Periodic.cpp
    Primitives.cpp
    Scheduler.cpp
    Settings.cpp
    StorageParent.cpp
    ThreadPool.cpp
    Wallet.cpp
    ZMQ.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Context.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Core.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Editor.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Endpoints.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Factory.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/HDSeed.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Periodic.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Primitives.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Settings.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/ThreadPool.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Wallet.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Legacy.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/Api.hpp"
    Context.hpp
    Core.hpp
    Endpoints.hpp
    Factory.hpp
    HDSeed.hpp
    Legacy.hpp
    Log.hpp
    Periodic.hpp
    Primitives.hpp
    Scheduler.hpp
    Settings.hpp
    StorageParent.hpp
    ThreadPool.hpp
    Wallet.hpp
    ZMQ.hpp
)

add_library(opentxs-api OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-api SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_link_libraries(opentxs-api PRIVATE Boost::headers opentxs::messages)
add_dependencies(opentxs-api generated_code)

if(OT_ENABLE_RPC)
  target_compile_definitions(opentxs-api PRIVATE OT_RPC=1)
else()
  target_compile_definitions(opentxs-api PRIVATE OT_RPC=0)
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api
    COMPONENT dev
  )
endif()

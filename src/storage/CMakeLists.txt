# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(drivers)
add_subdirectory(tree)

add_library(
  opentxs-storage OBJECT
  "Config.cpp"
  "Config.hpp"
  "Plugin.cpp"
  "Plugin.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/storage/Driver.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/storage/Plugin.hpp"
)
target_link_libraries(opentxs-storage PRIVATE opentxs::messages)
target_sources(opentxs-storage PRIVATE ${cxx-install-headers})

if(LMDB_EXPORT)
  target_sources(opentxs-storage PRIVATE "config/DefaultLMDB.cpp")
elseif(SQLITE_EXPORT)
  target_sources(opentxs-storage PRIVATE "config/DefaultSqlite.cpp")
elseif(FS_EXPORT)
  target_sources(opentxs-storage PRIVATE "config/DefaultFS.cpp")
else()
  target_sources(opentxs-storage PRIVATE "config/DefaultNone.cpp")
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-storage>)
add_dependencies(opentxs-storage generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/storage
    COMPONENT dev
  )
endif()

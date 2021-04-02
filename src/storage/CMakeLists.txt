# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(drivers)
add_subdirectory(tree)

add_library(
  opentxs-storage OBJECT
  "Plugin.cpp"
  "Plugin.hpp"
  "StorageConfig.cpp"
  "StorageConfig.hpp"
)

target_link_libraries(opentxs-storage PRIVATE opentxs::messages)

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

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(drivers)
add_subdirectory(tree)

target_sources(
  opentxs-common
  PRIVATE
    "Config.cpp"
    "Config.hpp"
    "Plugin.cpp"
    "Plugin.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/util/storage/Driver.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/storage/Plugin.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(LMDB_EXPORT)
  target_sources(opentxs-common PRIVATE "config/DefaultLMDB.cpp")
elseif(SQLITE_EXPORT)
  target_sources(opentxs-common PRIVATE "config/DefaultSqlite.cpp")
elseif(FS_EXPORT)
  target_sources(opentxs-common PRIVATE "config/DefaultFS.cpp")
else()
  target_sources(opentxs-common PRIVATE "config/DefaultNone.cpp")
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/util/storage
    COMPONENT dev
  )
endif()

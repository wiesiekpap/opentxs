# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-storage-drivers-sqlite OBJECT "")
target_link_libraries(opentxs-storage-drivers-sqlite PRIVATE opentxs::messages)

if(SQLITE_EXPORT)
  target_sources(
    opentxs-storage-drivers-sqlite PRIVATE "Sqlite3.cpp" "Sqlite3.hpp"
  )
  target_link_libraries(opentxs-storage-drivers-sqlite PRIVATE SQLite::SQLite3)
  target_link_libraries(opentxs PUBLIC SQLite::SQLite3)
else()
  target_sources(opentxs-storage-drivers-sqlite PRIVATE "null.cpp")
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-storage-drivers-sqlite>)
add_dependencies(opentxs-storage-drivers-sqlite generated_code)

# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-storage-drivers-multiplex OBJECT "Multiplex.cpp" "Multiplex.hpp"
)
target_link_libraries(
  opentxs-storage-drivers-multiplex PRIVATE opentxs::messages
)

if(FS_EXPORT)
  target_sources(opentxs-storage-drivers-multiplex PRIVATE "fs.cpp")
  target_link_libraries(
    opentxs-storage-drivers-multiplex PRIVATE Boost::headers
  )
  target_link_libraries(opentxs PUBLIC Boost::filesystem)
else()
  target_sources(opentxs-storage-drivers-multiplex PRIVATE "nofs.cpp")
endif()

if(SQLITE_EXPORT)
  target_sources(opentxs-storage-drivers-multiplex PRIVATE "sqlite.cpp")
  target_link_libraries(
    opentxs-storage-drivers-multiplex PRIVATE SQLite::SQLite3
  )
  target_link_libraries(opentxs PUBLIC SQLite::SQLite3)
else()
  target_sources(opentxs-storage-drivers-multiplex PRIVATE "nosqlite.cpp")
endif()

if(LMDB_EXPORT)
  target_sources(opentxs-storage-drivers-multiplex PRIVATE "lmdb.cpp")
  target_link_libraries(opentxs-storage-drivers-multiplex PRIVATE lmdb)
  target_link_libraries(opentxs PUBLIC lmdb)
else()
  target_sources(opentxs-storage-drivers-multiplex PRIVATE "nolmdb.cpp")
endif()

target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-storage-drivers-multiplex>
)
add_dependencies(opentxs-storage-drivers-multiplex generated_code)

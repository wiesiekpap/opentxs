# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-storage-drivers OBJECT
  "StorageMemDB.cpp"
  "StorageMemDB.hpp"
  "StorageMultiplex.cpp"
  "StorageMultiplex.hpp"
)
target_link_libraries(opentxs-storage-drivers PRIVATE opentxs::messages)

if(FS_EXPORT)
  target_sources(
    opentxs-storage-drivers
    PRIVATE
      "multiplex/fs.cpp"
      "StorageFS.cpp"
      "StorageFS.hpp"
      "StorageFSArchive.cpp"
      "StorageFSArchive.hpp"
      "StorageFSGC.cpp"
      "StorageFSGC.hpp"
  )
  target_link_libraries(opentxs-storage-drivers PRIVATE Boost::headers)
  target_link_libraries(opentxs PUBLIC Boost::filesystem)
else()
  target_sources(
    opentxs-storage-drivers PRIVATE "multiplex/nofs.cpp" "StorageFS_null.cpp"
  )
endif()

if(SQLITE_EXPORT)
  target_sources(
    opentxs-storage-drivers
    PRIVATE
      "multiplex/sqlite.cpp"
      "StorageSqlite3.cpp"
      "StorageSqlite3.hpp"
  )
  target_link_libraries(opentxs-storage-drivers PRIVATE SQLite::SQLite3)
  target_link_libraries(opentxs PUBLIC SQLite::SQLite3)
else()
  target_sources(
    opentxs-storage-drivers PRIVATE "multiplex/nosqlite.cpp"
                                    "StorageSqlite3_null.cpp"
  )
endif()

if(LMDB_EXPORT)
  target_sources(
    opentxs-storage-drivers
    PRIVATE
      "multiplex/lmdb.cpp"
      "StorageLMDB.cpp"
      "StorageLMDB.hpp"
  )
  target_link_libraries(opentxs-storage-drivers PRIVATE lmdb)
  target_link_libraries(opentxs PUBLIC lmdb)
else()
  target_sources(
    opentxs-storage-drivers PRIVATE "multiplex/nolmdb.cpp"
                                    "StorageLMDB_null.cpp"
  )
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-storage-drivers>)
add_dependencies(opentxs-storage-drivers generated_code)

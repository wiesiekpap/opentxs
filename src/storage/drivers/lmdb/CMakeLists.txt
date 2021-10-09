# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-storage-drivers-lmdb OBJECT "")
target_link_libraries(opentxs-storage-drivers-lmdb PRIVATE opentxs::messages)

if(LMDB_EXPORT)
  target_sources(opentxs-storage-drivers-lmdb PRIVATE "LMDB.cpp" "LMDB.hpp")
  target_link_libraries(opentxs-storage-drivers-lmdb PRIVATE lmdb)
  target_link_libraries(opentxs PUBLIC lmdb)
else()
  target_sources(opentxs-storage-drivers-lmdb PRIVATE "null.cpp")
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-storage-drivers-lmdb>)
add_dependencies(opentxs-storage-drivers-lmdb generated_code)

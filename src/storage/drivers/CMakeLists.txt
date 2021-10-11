# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(filesystem)
add_subdirectory(lmdb)
add_subdirectory(memdb)
add_subdirectory(multiplex)
add_subdirectory(sqlite)

add_library(
  opentxs-storage-drivers OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/storage/drivers/Drivers.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/storage/drivers/Factory.hpp"
  "Drivers.cpp"
)
target_link_libraries(opentxs-storage-drivers PRIVATE opentxs::messages)

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-storage-drivers>)
add_dependencies(opentxs-storage-drivers generated_code)

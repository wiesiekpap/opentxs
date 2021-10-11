# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-storage-drivers-filesystem OBJECT)
target_link_libraries(
  opentxs-storage-drivers-filesystem PRIVATE opentxs::messages
)

if(FS_EXPORT)
  target_sources(
    opentxs-storage-drivers-filesystem
    PRIVATE
      "Archiving.cpp"
      "Archiving.hpp"
      "Common.cpp"
      "Common.hpp"
      "GarbageCollected.cpp"
      "GarbageCollected.hpp"
  )
  target_link_libraries(
    opentxs-storage-drivers-filesystem PRIVATE Boost::headers
  )
  target_link_libraries(opentxs PUBLIC Boost::filesystem)
else()
  target_sources(opentxs-storage-drivers-filesystem PRIVATE "null.cpp")
endif()

target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-storage-drivers-filesystem>
)
add_dependencies(opentxs-storage-drivers-filesystem generated_code)

# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

#
# Define everything here that all wrappers will use, then process each language
# in its own subdirectory
#

# Locate include files for our wrappers
set(SWIG_INCLUDE_DIRS
    SYSTEM
    "${opentxs_SOURCE_DIR}/deps/"
    "${opentxs_SOURCE_DIR}/include/"
    "${opentxs_SOURCE_DIR}/include/opentxs"
    "${opentxs_SOURCE_DIR}/include/opentxs/core"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/crypto"
    "${opentxs_SOURCE_DIR}/include/opentxs/ext"
    "${opentxs_SOURCE_DIR}/include/opentxs/client"
)

# The opentxs libraries will we link to
set(OTX_LINK_LIBS opentxs)

if(CMAKE_CROSSCOMPILING)
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM_PRESERVE
      "${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}"
  )
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE_PRESERVE
      "${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}"
  )
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
endif()

find_package(
  SWIG
  4.0.0
  REQUIRED
)

if(CMAKE_CROSSCOMPILING)
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM
      "${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM_PRESERVE}"
  )
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE
      "${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE_PRESERVE}"
  )
endif()

include(${SWIG_USE_FILE})
include_directories(${SWIG_INCLUDE_DIRS})

if(OT_WITH_JAVA)
  add_subdirectory(java)
endif()

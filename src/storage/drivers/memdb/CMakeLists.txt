# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-storage-drivers-memdb OBJECT "MemDB.cpp" "MemDB.hpp")
target_link_libraries(opentxs-storage-drivers-memdb PRIVATE opentxs::messages)

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-storage-drivers-memdb>)
add_dependencies(opentxs-storage-drivers-memdb generated_code)

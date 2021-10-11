# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-api-storage OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/api/storage/Factory.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/api/storage/Storage.hpp"
  "Storage.cpp"
  "Storage.hpp"
)
target_link_libraries(opentxs-api-storage PRIVATE opentxs::messages)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-storage>)
add_dependencies(opentxs-api-storage generated_code)

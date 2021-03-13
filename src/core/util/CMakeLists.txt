# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-core-util OBJECT "Common.cpp" "Tag.cpp")
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/util/Common.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/util/Tag.hpp"
)
target_link_libraries(opentxs-core-util PRIVATE opentxs::messages)
target_sources(opentxs-core-util PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-core-util>)
add_dependencies(opentxs-core-util generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/util
    COMPONENT dev
  )
endif()

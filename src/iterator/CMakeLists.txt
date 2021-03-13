# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-iterator OBJECT "")
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/iterator/Bidirectional.hpp"
)
target_link_libraries(opentxs-iterator PRIVATE opentxs::messages)
set_target_properties(opentxs-iterator PROPERTIES LINKER_LANGUAGE CXX)
target_sources(opentxs-iterator PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-iterator>)
add_dependencies(opentxs-iterator generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/iterator
    COMPONENT dev
  )
endif()

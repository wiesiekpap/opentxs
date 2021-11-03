# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-core-script OBJECT "OTScriptable.cpp")
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTScriptable.hpp"
)
target_link_libraries(opentxs-core-script PRIVATE opentxs::messages)
target_include_directories(
  opentxs-core-script SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/ChaiScript/include"
          "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-core-script PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-core-script>)
add_dependencies(opentxs-core-script generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/script
    COMPONENT dev
  )
endif()

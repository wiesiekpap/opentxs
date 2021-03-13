# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-client OBJECT
  ${cxx-sources}
  ${cxx-headers}
  "NymData.cpp"
  "OTAPI_Exec.cpp"
  "OTAPI_Func.cpp"
  "OTAPI_Func.hpp"
  "OTClient.cpp"
  "OT_API.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/client/NymData.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/client/OTAPI_Exec.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/client/OT_API.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/client/OTClient.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/client/ServerAction.hpp"
)
target_link_libraries(opentxs-client PRIVATE opentxs::messages)
target_include_directories(
  opentxs-client SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-client PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-client>)
add_dependencies(opentxs-client generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/client
    COMPONENT dev
  )
endif()

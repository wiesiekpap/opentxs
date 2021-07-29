# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-api-client-activity OBJECT
  "Activity.cpp"
  "Imp.cpp"
  "Imp.hpp"
  "MailCache.cpp"
  "MailCache.hpp"
)

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/Activity.hpp"
)

target_link_libraries(opentxs-api-client-activity PRIVATE opentxs::messages)
target_include_directories(
  opentxs-api-client-activity SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

target_sources(opentxs-api-client-activity PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-client-activity>)
add_dependencies(opentxs-api-client-activity generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/client
    COMPONENT dev
  )
endif()

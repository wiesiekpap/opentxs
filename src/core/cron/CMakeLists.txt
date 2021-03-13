# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-core-cron OBJECT "OTCron.cpp" "OTCronItem.cpp")
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/cron/OTCron.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/cron/OTCronItem.hpp"
)
target_link_libraries(opentxs-core-cron PRIVATE opentxs::messages)
target_include_directories(
  opentxs-core-cron SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-core-cron PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-core-cron>)
add_dependencies(opentxs-core-cron generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/cron
    COMPONENT dev
  )
endif()

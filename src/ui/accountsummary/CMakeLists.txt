# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-ui-accountsummary OBJECT
  "AccountSummary.cpp"
  "AccountSummary.hpp"
  "AccountSummaryItem.cpp"
  "AccountSummaryItem.hpp"
  "IssuerItem.cpp"
  "IssuerItem.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountSummary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountSummaryItem.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/IssuerItem.hpp"
)
target_link_libraries(opentxs-ui-accountsummary PRIVATE opentxs::messages)
target_include_directories(
  opentxs-ui-accountsummary SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-ui-accountsummary PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-accountsummary>)
add_dependencies(opentxs-ui-accountsummary generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()

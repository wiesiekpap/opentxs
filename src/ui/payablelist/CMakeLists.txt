# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-ui-payablelist OBJECT
  "PayableList.cpp"
  "PayableList.hpp"
  "PayableListItem.cpp"
  "PayableListItem.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/PayableList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/PayableListItem.hpp"
)
target_link_libraries(opentxs-ui-payablelist PRIVATE opentxs::messages)
target_sources(opentxs-ui-payablelist PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-payablelist>)
add_dependencies(opentxs-ui-payablelist generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()

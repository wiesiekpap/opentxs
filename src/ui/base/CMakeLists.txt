# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-ui-base OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/ui/UI.hpp"
  "Combined.hpp"
  "Items.hpp"
  "List.hpp"
  "Row.hpp"
  "RowType.hpp"
  "Widget.cpp"
  "Widget.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/List.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ListRow.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/Widget.hpp"
)
target_link_libraries(opentxs-ui-base PRIVATE opentxs::messages)
target_sources(opentxs-ui-base PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-base>)
add_dependencies(opentxs-ui-base generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-api-client-ui OBJECT
  "Imp-base.cpp"
  "Imp-base.hpp"
  "UI.cpp"
  "UpdateManager.cpp"
  "UpdateManager.hpp"
)
target_link_libraries(opentxs-api-client-ui PRIVATE opentxs::messages)

if(OT_QT_EXPORT)
  target_sources(
    opentxs-api-client-ui
    PRIVATE
      "Imp-qt.cpp"
      "Imp-qt.hpp"
      "Qt.cpp"
  )
  target_link_libraries(opentxs-api-client-ui PRIVATE Qt5::Core Qt5::Gui)
  target_link_libraries(opentxs PRIVATE Qt5::Core Qt5::Gui)
else()
  target_sources(opentxs-api-client-ui PRIVATE "No_Qt.cpp")
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-client-ui>)
add_dependencies(opentxs-api-client-ui generated_code)

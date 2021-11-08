# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "Imp-base.cpp"
    "Imp-base.hpp"
    "UI.cpp"
    "UpdateManager.cpp"
    "UpdateManager.hpp"
)

if(OT_QT_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "Imp-qt.cpp"
      "Imp-qt.hpp"
      "Qt.cpp"
  )
  target_link_libraries(opentxs-common PRIVATE Qt5::Core Qt5::Gui)
  target_link_libraries(opentxs PUBLIC Qt5::Core Qt5::Gui)
else()
  target_sources(opentxs-common PRIVATE "No_Qt.cpp")
endif()

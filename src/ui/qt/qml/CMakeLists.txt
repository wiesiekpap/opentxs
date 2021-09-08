# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_link_libraries(opentxs-ui-qt PRIVATE opentxs::messages)

if(OT_QML_EXPORT)
  target_sources(opentxs-ui-qt PRIVATE "Qml.cpp")
  target_link_libraries(opentxs-ui-qt PRIVATE Qt5::Qml)
  target_link_libraries(opentxs PUBLIC Qt5::Qml)
else()
  target_sources(opentxs-ui-qt PRIVATE "Null.cpp")
endif()

if(OT_QT_EXPORT)
  target_link_libraries(opentxs-ui-qt PRIVATE Qt5::Core)
  target_link_libraries(opentxs PUBLIC Qt5::Core)
endif()

add_dependencies(opentxs-ui-qt generated_code)

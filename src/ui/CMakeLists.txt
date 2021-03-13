# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(opentxs_set_qt target_name)
  if(OT_WITH_QT)
    target_link_libraries(${target_name} PRIVATE Qt5::Core)
    target_link_libraries(opentxs PUBLIC Qt5::Core)

    if(OT_QML_EXPORT)
      target_link_libraries(${target_name} PRIVATE Qt5::Qml)
      target_link_libraries(opentxs PUBLIC Qt5::Qml)
      target_compile_definitions(${target_name} PRIVATE -DOT_WITH_QML=1)
    else()
      target_compile_definitions(${target_name} PRIVATE -DOT_WITH_QML=0)
    endif()
  else()
    target_compile_definitions(${target_name} PRIVATE -DOT_WITH_QML=0)
  endif()
endmacro()

add_subdirectory(accountactivity)
add_subdirectory(accountlist)
add_subdirectory(accountsummary)
add_subdirectory(activitysummary)
add_subdirectory(activitythread)
add_subdirectory(base)

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(blockchainselection)
endif()

add_subdirectory(contact)
add_subdirectory(contactlist)

if(OT_QT_EXPORT)
  add_subdirectory(displayscale)
  add_subdirectory(seedvalidator)
endif()

add_subdirectory(messagablelist)
add_subdirectory(payablelist)
add_subdirectory(profile)
add_subdirectory(unitlist)

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/Blockchains.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/Types.hpp"
)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources BlockchainSelection.cpp BlockchainSelectionItem.cpp)
set(ot-qobject-install-headers "")
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/BlockchainSelection.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/BlockchainSelectionItem.hpp"
)
set(cxx-headers ${cxx-install-headers} BlockchainSelection.hpp
                BlockchainSelectionItem.hpp
)

if(OT_WITH_QT)
  list(
    APPEND ot-qobject-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/BlockchainSelection.hpp"
  )
  qt5_wrap_cpp(MOC_SRCS ${ot-qobject-install-headers})
else()
  set(MOC_SRCS "")
endif()

add_library(
  opentxs-ui-blockchainselection OBJECT
  ${cxx-sources}
  ${cxx-headers}
  ${MOC_SRCS}
  ${ot-qobject-install-headers}
)
target_link_libraries(opentxs-ui-blockchainselection PRIVATE opentxs::messages)

if(OT_QT_EXPORT)
  target_link_libraries(opentxs-ui-blockchainselection PUBLIC Qt5::Core)

  if(OT_QML_EXPORT)
    target_link_libraries(opentxs-ui-blockchainselection PUBLIC Qt5::Qml)
    target_compile_definitions(
      opentxs-ui-blockchainselection PRIVATE -DOT_WITH_QML=1
    )
  else()
    target_compile_definitions(
      opentxs-ui-blockchainselection PRIVATE -DOT_WITH_QML=0
    )
  endif()
else()
  target_compile_definitions(
    opentxs-ui-blockchainselection PRIVATE -DOT_WITH_QML=0
  )
endif()

add_dependencies(opentxs-ui-blockchainselection generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
  if(OT_QT_EXPORT)
    install(
      FILES ${ot-qobject-install-headers}
      DESTINATION include/opentxs/ui/qt
      COMPONENT dev
    )
  endif()
endif()

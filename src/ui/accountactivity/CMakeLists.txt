# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    AccountActivity.cpp
    BalanceItem.cpp
    ChequeBalanceItem.cpp
    CustodialAccountActivity.cpp
    TransferBalanceItem.cpp
)
set(ot-qobject-install-headers "")
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountActivity.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/BalanceItem.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    AccountActivity.hpp
    BalanceItem.hpp
    ChequeBalanceItem.hpp
    CustodialAccountActivity.hpp
    TransferBalanceItem.hpp
)

if(OT_BLOCKCHAIN_EXPORT)
  list(
    APPEND
    cxx-sources
    BlockchainAccountActivity.cpp
    BlockchainBalanceItem.cpp
  )
  list(
    APPEND
    cxx-headers
    BlockchainAccountActivity.hpp
    BlockchainBalanceItem.hpp
  )
endif()

if(OT_WITH_QT)
  list(
    APPEND
    ot-qobject-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/AccountActivity.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/AmountValidator.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/DestinationValidator.hpp"
  )
  list(
    APPEND
    cxx-sources
    AmountValidator.cpp
    BlockchainDestinationValidator.cpp
    CustodialDestinationValidator.cpp
    DestinationValidator.cpp
  )
  list(
    APPEND
    cxx-headers
    AmountValidator.hpp
    DestinationValidator.hpp
  )
  qt5_wrap_cpp(MOC_SRCS ${ot-qobject-install-headers})
else()
  set(MOC_SRCS "")
endif()

add_library(
  opentxs-ui-accountactivity OBJECT
  ${cxx-sources}
  ${cxx-headers}
  ${MOC_SRCS}
  ${ot-qobject-install-headers}
)
target_link_libraries(opentxs-ui-accountactivity PRIVATE opentxs::messages)
target_include_directories(
  opentxs-ui-accountactivity SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OT_QT_EXPORT)
  target_link_libraries(opentxs-ui-accountactivity PUBLIC Qt5::Core Qt5::Gui)

  if(OT_QML_EXPORT)
    target_link_libraries(opentxs-ui-accountactivity PUBLIC Qt5::Qml)
    target_compile_definitions(
      opentxs-ui-accountactivity PRIVATE -DOT_WITH_QML=1
    )
  else()
    target_compile_definitions(
      opentxs-ui-accountactivity PRIVATE -DOT_WITH_QML=0
    )
  endif()
else()
  target_compile_definitions(opentxs-ui-accountactivity PRIVATE -DOT_WITH_QML=0)
endif()

add_dependencies(opentxs-ui-accountactivity generated_code)

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

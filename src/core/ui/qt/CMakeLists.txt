# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_QT_EXPORT)
  add_subdirectory(displayscale)
  add_subdirectory(seedvalidator)
endif()

add_subdirectory(qml)

if(OT_QT_EXPORT)
  list(
    APPEND
    OPENTXS_QT_PUBLIC_HEADERS
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/AccountActivity.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/AccountList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/AccountSummary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/ActivitySummary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/ActivityThread.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/AmountValidator.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/BlankModel.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/BlockchainAccountStatus.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/BlockchainSelection.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/BlockchainStatistics.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/Contact.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/ContactList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/DestinationValidator.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/MessagableList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/Model.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/PayableList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/Profile.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/qt/UnitList.hpp"
  )
  list(
    APPEND
    OPENTXS_QT_PRIVATE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/DraftValidator.hpp"
  )
  target_sources(
    opentxs-common
    PRIVATE
      "AccountActivity.cpp"
      "AccountList.cpp"
      "AccountSummary.cpp"
      "ActivitySummary.cpp"
      "ActivityThread.cpp"
      "AmountValidator.cpp"
      "AmountValidator.hpp"
      "BlockchainAccountStatus.cpp"
      "BlockchainDestinationValidator.cpp"
      "BlockchainSelection.cpp"
      "BlockchainStatistics.cpp"
      "Contact.cpp"
      "ContactList.cpp"
      "CustodialDestinationValidator.cpp"
      "DestinationValidator.cpp"
      "DestinationValidator.hpp"
      "DraftValidator.cpp"
      "MessagableList.cpp"
      "Model.cpp"
      "PayableList.cpp"
      "Profile.cpp"
      "SendMonitor.cpp"
      "SendMonitor.hpp"
      "UnitList.cpp"
      ${MOC_SRCS}
  )

  if(OT_BLOCKCHAIN_EXPORT)
    target_sources(opentxs-common PRIVATE "Blockchain.cpp")
  endif()

  target_link_libraries(opentxs-common PRIVATE Qt::Core Qt::Gui)
  target_link_libraries(opentxs PUBLIC Qt::Core Qt::Gui)
else()
  target_sources(opentxs-common PRIVATE "no-Qt.cpp")
endif()

set(OPENTXS_QT_PUBLIC_HEADERS
    ${OPENTXS_QT_PUBLIC_HEADERS}
    PARENT_SCOPE
)
set(OPENTXS_QT_PRIVATE_HEADERS
    ${OPENTXS_QT_PRIVATE_HEADERS}
    PARENT_SCOPE
)

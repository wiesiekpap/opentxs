# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-ui-qt OBJECT "")

if(OT_QT_EXPORT)
  add_subdirectory(displayscale)
  add_subdirectory(seedvalidator)
endif()

add_subdirectory(qml)

if(OT_QT_EXPORT)
  set(ot-qobject-install-headers
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/AccountActivity.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/AccountList.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/AccountSummary.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/ActivitySummary.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/ActivityThread.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/AmountValidator.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/BlankModel.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/BlockchainAccountStatus.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/BlockchainSelection.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/BlockchainStatistics.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/Contact.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/ContactList.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/DestinationValidator.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/MessagableList.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/Model.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/PayableList.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/Profile.hpp"
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/UnitList.hpp"
  )
  qt5_wrap_cpp(
    MOC_SRCS
    ${ot-qobject-install-headers}
    "DraftValidator.hpp"
  )
  target_sources(
    opentxs-ui-qt
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
    target_sources(opentxs-ui-qt PRIVATE "Blockchain.cpp")
  endif()

  target_link_libraries(opentxs-ui-qt PRIVATE Qt5::Core Qt5::Gui)
  target_link_libraries(opentxs PUBLIC Qt5::Core Qt5::Gui)
  target_sources(opentxs-ui-qt PRIVATE ${cxx-install-headers})
  add_dependencies(opentxs-ui-qt generated_code)

  if(OPENTXS_STANDALONE)
    install(
      FILES ${ot-qobject-install-headers}
      DESTINATION include/opentxs/ui/qt
      COMPONENT dev
    )
  endif()
else()
  target_sources(opentxs-ui-qt PRIVATE "no-Qt.cpp")
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-qt>)

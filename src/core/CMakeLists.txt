# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(contract)
add_subdirectory(cron)
add_subdirectory(crypto)
add_subdirectory(identifier)
add_subdirectory(otprotob)
add_subdirectory(recurring)
add_subdirectory(script)
add_subdirectory(trade)
add_subdirectory(transaction)
add_subdirectory(util)
add_subdirectory(paymentcode)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/include/opentxs/core/UniqueQueue.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/Core.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/Null.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/identifier/Identifier.hpp"
    "Account.cpp"
    "AccountVisitor.cpp"
    "Amount.cpp"
    "Armored.cpp"
    "Armored.hpp"
    "Cheque.cpp"
    "Contract.cpp"
    "Core.cpp"
    "Data.cpp"
    "Data.hpp"
    "Flag.cpp"
    "Flag.hpp"
    "Identifier.cpp"
    "Identifier.hpp"
    "Instrument.cpp"
    "Item.cpp"
    "Ledger.cpp"
    "Message.cpp"
    "NumList.cpp"
    "NymFile.cpp"
    "NymFile.hpp"
    "OTStorage.cpp"
    "OTTrackable.cpp"
    "OTTransaction.cpp"
    "OTTransactionType.cpp"
    "PasswordPrompt.cpp"
    "Secret.cpp"
    "Secret.hpp"
    "Shutdown.cpp"
    "Shutdown.hpp"
    "StateMachine.cpp"
    "StateMachine.hpp"
    "String.cpp"
    "String.hpp"
    "StringXML.cpp"
    "StringXML.hpp"
    "Worker.hpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Account.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/AccountVisitor.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/AddressType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Amount.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Armored.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Cheque.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Contract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Data.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Editor.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Flag.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Helpers.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Identifier.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Instrument.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Item.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Ledger.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Lockable.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Message.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/NumList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/NymFile.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/OTTrackable.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/OTTransaction.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/OTTransactionType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/PasswordPrompt.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Secret.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/String.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/StringXML.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Types.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/UnitType.hpp"
)
target_link_libraries(
  opentxs-common
  PRIVATE
    ZLIB::ZLIB
    opentxs-core-otprotob
    unofficial-sodium::sodium
)
target_link_libraries(opentxs PUBLIC ZLIB::ZLIB unofficial-sodium::sodium)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core
    COMPONENT dev
  )
endif()

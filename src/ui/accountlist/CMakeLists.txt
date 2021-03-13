# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-ui-accountlist OBJECT
  "AccountList.cpp"
  "AccountList.hpp"
  "AccountListItem.cpp"
  "AccountListItem.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountListItem.hpp"
)
target_link_libraries(opentxs-ui-accountlist PRIVATE opentxs::messages)
target_include_directories(
  opentxs-ui-accountlist SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-ui-accountlist PRIVATE "BlockchainAccountListItem.cpp"
                                   "BlockchainAccountListItem.hpp"
  )
endif()

if(OT_WITH_QT)
  set(ot-qobject-install-headers
      "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/AccountList.hpp"
  )
  qt5_wrap_cpp(MOC_SRCS ${ot-qobject-install-headers})
  target_sources(opentxs-ui-accountlist PRIVATE ${MOC_SRCS})

  if(OPENTXS_STANDALONE)
    install(
      FILES ${ot-qobject-install-headers}
      DESTINATION include/opentxs/ui/qt
      COMPONENT dev
    )
  endif()
endif()

opentxs_set_qt(opentxs-ui-accountlist)
target_sources(opentxs-ui-accountlist PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-accountlist>)
add_dependencies(opentxs-ui-accountlist generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()

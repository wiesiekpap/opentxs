# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-ui-activitythread OBJECT
  "ActivityThread.cpp"
  "ActivityThread.hpp"
  "ActivityThreadItem.cpp"
  "ActivityThreadItem.hpp"
  "MailItem.cpp"
  "MailItem.hpp"
  "PaymentItem.cpp"
  "PaymentItem.hpp"
  "PendingSend.cpp"
  "PendingSend.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ActivityThread.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ActivityThreadItem.hpp"
)
target_link_libraries(opentxs-ui-activitythread PRIVATE opentxs::messages)
target_include_directories(
  opentxs-ui-activitythread SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-ui-activitythread PRIVATE "BlockchainActivityThreadItem.cpp"
                                      "BlockchainActivityThreadItem.hpp"
  )
endif()

target_sources(opentxs-ui-activitythread PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-activitythread>)
add_dependencies(opentxs-ui-activitythread generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()

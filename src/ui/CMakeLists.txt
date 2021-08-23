# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(accountactivity)
add_subdirectory(accountlist)
add_subdirectory(accountsummary)
add_subdirectory(activitysummary)
add_subdirectory(activitythread)
add_subdirectory(base)
add_subdirectory(blockchainaccountstatus)
add_subdirectory(blockchainselection)
add_subdirectory(blockchainstatistics)
add_subdirectory(contact)
add_subdirectory(contactlist)
add_subdirectory(messagablelist)
add_subdirectory(payablelist)
add_subdirectory(profile)
add_subdirectory(qt)
add_subdirectory(unitlist)

add_library(
  opentxs-ui OBJECT "${opentxs_SOURCE_DIR}/src/internal/ui/UI.hpp" "UI.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/Blockchains.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/Types.hpp"
)
target_link_libraries(opentxs-ui PRIVATE opentxs::messages)
target_sources(opentxs-ui PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui>)
add_dependencies(opentxs-ui generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()

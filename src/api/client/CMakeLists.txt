# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(activity)
add_subdirectory(ui)

add_library(
  opentxs-api-client OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/api/client/Client.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/api/client/Factory.hpp"
  "Client.cpp"
  "Contacts.cpp"
  "Contacts.hpp"
  "Issuer.cpp"
  "Issuer.hpp"
  "OTX.cpp"
  "OTX.hpp"
  "Pair.cpp"
  "Pair.hpp"
  "ServerAction.cpp"
  "ServerAction.hpp"
  "Workflow.cpp"
  "Workflow.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/Contacts.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/Issuer.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/OTX.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/Pair.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/PaymentWorkflowState.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/PaymentWorkflowType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/ServerAction.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/Types.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/UI.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/client/Workflow.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/session/Notary.hpp"
)
target_link_libraries(opentxs-api-client PRIVATE Boost::headers opentxs-common)
target_include_directories(
  opentxs-api-client SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

target_sources(opentxs-api-client PRIVATE ${cxx-install-headers})

if(NOT OT_IWYU)
  target_precompile_headers(
    opentxs-api-client
    REUSE_FROM
    opentxs-common
  )
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-client>)
add_dependencies(opentxs-api-client generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/client
    COMPONENT dev
  )
endif()

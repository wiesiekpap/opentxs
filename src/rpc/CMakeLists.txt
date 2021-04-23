# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_ENABLE_RPC)
  add_subdirectory(processor)
endif()

add_subdirectory(request)
add_subdirectory(response)

add_library(
  opentxs-rpc OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/rpc/RPC.hpp"
  "AccountData.cpp"
  "AccountEvent.cpp"
  "Common.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/AccountData.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/AccountEvent.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/AccountEventType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/AccountType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/CommandType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/ContactEventType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/PaymentType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/PushType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/ResponseCode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/Types.hpp"
)
target_link_libraries(opentxs-rpc PRIVATE Boost::headers opentxs::messages)
target_include_directories(
  opentxs-rpc SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OT_ENABLE_RPC)
  target_sources(
    opentxs-rpc
    PRIVATE
      "RPC.cpp"
      "RPC.hpp"
      "RPC.tpp"
  )
else()
  target_sources(opentxs-rpc PRIVATE "Null.cpp")
endif()

target_sources(opentxs-rpc PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-rpc>)
add_dependencies(opentxs-rpc generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/rpc
    COMPONENT dev
  )
endif()

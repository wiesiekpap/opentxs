# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_ENABLE_RPC)
  add_subdirectory(processor)
endif()

add_subdirectory(request)
add_subdirectory(response)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/interface/rpc/RPC.hpp"
    "AccountData.cpp"
    "AccountEvent.cpp"
    "Common.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/rpc/AccountData.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/rpc/AccountEvent.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/rpc/AccountEventType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/rpc/AccountType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/rpc/CommandType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/rpc/ContactEventType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/rpc/PaymentType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/rpc/PushType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/rpc/ResponseCode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/rpc/Types.hpp"
)

if(OT_ENABLE_RPC)
  target_sources(
    opentxs-common
    PRIVATE
      "RPC.cpp"
      "RPC.hpp"
      "RPC.tpp"
  )
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/interface/rpc
    COMPONENT dev
  )
endif()

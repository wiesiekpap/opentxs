# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-rpc-response OBJECT
  "Base.cpp"
  "Base.hpp"
  "Factory.cpp"
  "GetAccountActivity.cpp"
  "GetAccountBalance.cpp"
  "Invalid.cpp"
  "Invalid.hpp"
  "ListAccounts.cpp"
  "ListNyms.cpp"
  "SendPayment.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/response/Base.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/response/GetAccountActivity.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/response/GetAccountBalance.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/response/ListAccounts.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/response/ListNyms.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/response/SendPayment.hpp"
)
target_link_libraries(opentxs-rpc-response PRIVATE opentxs::messages)
target_sources(opentxs-rpc-response PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-rpc-response>)
add_dependencies(opentxs-rpc-response generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/rpc/response
    COMPONENT dev
  )
endif()

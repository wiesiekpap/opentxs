# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-rpc-request OBJECT
  "Base.cpp"
  "Base.hpp"
  "Factory.cpp"
  "GetAccountActivity.cpp"
  "GetAccountBalance.cpp"
  "ListAccounts.cpp"
  "ListNyms.cpp"
  "SendPayment.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/request/Base.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/request/GetAccountActivity.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/request/GetAccountBalance.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/request/ListAccounts.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/request/ListNyms.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/rpc/request/SendPayment.hpp"
)
target_link_libraries(opentxs-rpc-request PRIVATE opentxs::messages)
target_sources(opentxs-rpc-request PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-rpc-request>)
add_dependencies(opentxs-rpc-request generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/rpc/request
    COMPONENT dev
  )
endif()

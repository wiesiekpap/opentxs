# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
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
    "${opentxs_SOURCE_DIR}/include/opentxs/util/rpc/request/Base.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/rpc/request/GetAccountActivity.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/rpc/request/GetAccountBalance.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/rpc/request/ListAccounts.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/rpc/request/ListNyms.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/rpc/request/SendPayment.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/util/rpc/request
    COMPONENT dev
  )
endif()

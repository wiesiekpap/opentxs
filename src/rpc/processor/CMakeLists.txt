# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-rpc-processor OBJECT
  "GetAccountActivity.cpp"
  "GetAccountBalance.cpp"
  "ListAccounts.cpp"
  "ListNyms.cpp"
  "SendPayment.cpp"
)
target_link_libraries(opentxs-rpc-processor PRIVATE opentxs::messages)
target_include_directories(
  opentxs-rpc-processor SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-rpc-processor>)
add_dependencies(opentxs-rpc-processor generated_code)

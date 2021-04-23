# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-api-context OBJECT "")

if(OT_ENABLE_RPC)
  target_sources(opentxs-api-context PRIVATE "rpc.cpp")
else()
  target_sources(opentxs-api-context PRIVATE "no-rpc.cpp")
endif()

target_link_libraries(opentxs-api-context PRIVATE opentxs::messages)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-context>)
add_dependencies(opentxs-api-context generated_code)

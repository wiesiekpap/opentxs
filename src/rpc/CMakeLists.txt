# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-rpc OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/rpc/RPC.hpp"
  "RPC.cpp"
  "RPC.hpp"
  "RPC.tpp"
)
target_link_libraries(opentxs-rpc PRIVATE opentxs::messages)
target_include_directories(
  opentxs-rpc SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-rpc>)
add_dependencies(opentxs-rpc generated_code)

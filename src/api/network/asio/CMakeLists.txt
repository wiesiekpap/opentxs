# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-api-network-asio OBJECT
  "Acceptor.cpp"
  "Acceptor.hpp"
  "Acceptors.cpp"
  "Acceptors.hpp"
  "Asio.cpp"
  "Buffers.cpp"
  "Buffers.hpp"
  "Context.cpp"
  "Context.hpp"
  "Imp.cpp"
  "Imp.hpp"
)
target_link_libraries(
  opentxs-api-network-asio
  PRIVATE
    opentxs::messages
    Boost::headers
    OpenSSL::SSL
)
target_link_libraries(
  opentxs
  PUBLIC
    Boost::system
    Boost::thread
    OpenSSL::SSL
)
target_include_directories(
  opentxs-api-network-asio SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-network-asio>)
add_dependencies(opentxs-api-network-asio generated_code)

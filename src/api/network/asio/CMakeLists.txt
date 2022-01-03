# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
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
target_link_libraries(opentxs-common PRIVATE OpenSSL::SSL)
target_link_libraries(
  opentxs
  PUBLIC
    Boost::system
    Boost::thread
    OpenSSL::SSL
)

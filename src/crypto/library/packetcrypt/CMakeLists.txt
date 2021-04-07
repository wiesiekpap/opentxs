# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-crypto-library-packetcrypt OBJECT "PacketCrypt.cpp" "PacketCrypt.hpp"
)
target_link_libraries(
  opentxs-crypto-library-packetcrypt
  PRIVATE
    Boost::headers
    opentxs::messages
    packetcrypt
)
set_target_properties(
  opentxs-crypto-library-packetcrypt PROPERTIES C_STANDARD 11
)

target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-crypto-library-packetcrypt>
)
add_dependencies(opentxs-crypto-library-packetcrypt generated_code)

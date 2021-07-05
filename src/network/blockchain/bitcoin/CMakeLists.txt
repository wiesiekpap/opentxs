# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-network-blockchain-bitcoin OBJECT "CompactSize.cpp" "CompactSize.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/bitcoin/CompactSize.hpp"
)
target_link_libraries(
  opentxs-network-blockchain-bitcoin PRIVATE opentxs::messages Boost::headers
)
target_link_libraries(opentxs PUBLIC Boost::system)
target_sources(
  opentxs-network-blockchain-bitcoin PRIVATE ${cxx-install-headers}
)
target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-network-blockchain-bitcoin>
)
add_dependencies(opentxs-network-blockchain-bitcoin generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/blockchain/bitcoin
    COMPONENT dev
  )
endif()

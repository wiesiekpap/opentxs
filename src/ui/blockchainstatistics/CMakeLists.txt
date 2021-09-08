# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-ui-blockchainstatistics OBJECT
  "BlockchainStatistics.cpp"
  "BlockchainStatistics.hpp"
  "BlockchainStatisticsItem.cpp"
  "BlockchainStatisticsItem.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/BlockchainStatistics.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/BlockchainStatisticsItem.hpp"
)
target_link_libraries(opentxs-ui-blockchainstatistics PRIVATE opentxs::messages)
target_sources(opentxs-ui-blockchainstatistics PRIVATE ${cxx-install-headers})
target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-blockchainstatistics>
)
add_dependencies(opentxs-ui-blockchainstatistics generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/FilterOracle.hpp"
    "BlockIndexer.cpp"
    "BlockIndexer.hpp"
    "FilterCheckpoints.hpp"
    "FilterDownloader.hpp"
    "HeaderDownloader.hpp"
    "FilterOracle.cpp"
    "FilterOracle.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/FilterOracle.hpp"
)
target_link_libraries(opentxs-common PRIVATE Boost::headers)
target_link_libraries(opentxs PUBLIC Boost::system)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/node
    COMPONENT dev
  )
endif()

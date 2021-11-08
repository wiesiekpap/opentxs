# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "Acknowledgement.cpp"
    "Base.cpp"
    "Base.hpp"
    "Block.cpp"
    "Common.cpp"
    "Data.cpp"
    "Factory.cpp"
    "Query.cpp"
    "Request.cpp"
    "State.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/sync/Acknowledgement.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/sync/Base.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/sync/Block.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/sync/Data.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/sync/MessageType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/sync/Query.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/sync/Request.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/sync/State.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/sync/Types.hpp"
)
target_link_libraries(opentxs PUBLIC Boost::system)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/blockchain/sync
    COMPONENT dev
  )
endif()

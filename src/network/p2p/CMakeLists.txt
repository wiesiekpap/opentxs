# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(server)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/p2p/Factory.hpp"
    "Acknowledgement.cpp"
    "Base.cpp"
    "Base.hpp"
    "Block.cpp"
    "Common.cpp"
    "Data.cpp"
    "Factory.cpp"
    "PublishContract.cpp"
    "PublishContractReply.cpp"
    "Query.cpp"
    "QueryContract.cpp"
    "QueryContractReply.cpp"
    "Request.cpp"
    "State.cpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/Acknowledgement.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/Base.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/Block.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/Data.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/MessageType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/PublishContract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/PublishContractReply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/Query.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/QueryContract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/QueryContractReply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/Request.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/State.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/p2p/Types.hpp"
)
target_link_libraries(opentxs PUBLIC Boost::system)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/p2p
    COMPONENT dev
  )
endif()

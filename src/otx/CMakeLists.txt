# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(blind)
add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(consensus)
add_subdirectory(server)
add_subdirectory(smartcontract)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/AccountList.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/OTX.hpp"
    "AccountList.cpp"
    "OTX.cpp"
    "Reply.cpp"
    "Reply.hpp"
    "Request.cpp"
    "Request.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/ConsensusType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/LastReplyStatus.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/OTXPushType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/OperationType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/Reply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/Request.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/ServerReplyType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/ServerRequestType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/otx
    COMPONENT dev
  )
endif()

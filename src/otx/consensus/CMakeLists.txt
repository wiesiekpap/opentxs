# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-otx-consensus OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/otx/consensus/Consensus.hpp"
  "Base.cpp"
  "Base.hpp"
  "Client.cpp"
  "Client.hpp"
  "ManagedNumber.cpp"
  "ManagedNumber.hpp"
  "Server.cpp"
  "Server.hpp"
  "TransactionStatement.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/consensus/Base.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/consensus/Client.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/consensus/ManagedNumber.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/consensus/Server.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/consensus/TransactionStatement.hpp"
)
target_link_libraries(opentxs-otx-consensus PRIVATE opentxs::messages)
target_include_directories(
  opentxs-otx-consensus SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-otx-consensus PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-otx-consensus>)
add_dependencies(opentxs-otx-consensus generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/otx/consensus
    COMPONENT dev
  )
endif()

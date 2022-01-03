# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
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
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/otx/consensus
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/identity/wot/verification/Verification.hpp"
    "Group.cpp"
    "Group.hpp"
    "Item.cpp"
    "Item.hpp"
    "Nym.cpp"
    "Nym.hpp"
    "Set.cpp"
    "Set.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/verification/Group.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/verification/Item.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/verification/Nym.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/verification/Set.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/identity/wot/verification
    COMPONENT dev
  )
endif()

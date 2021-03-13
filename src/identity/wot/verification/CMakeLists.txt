# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-identity-wot-verification OBJECT
  "Group.cpp"
  "Item.cpp"
  "Nym.cpp"
  "Set.cpp"
  "${opentxs_SOURCE_DIR}/src/internal/identity/wot/verification/Verification.hpp"
  "Group.hpp"
  "Item.hpp"
  "Nym.hpp"
  "Set.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/verification/Group.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/verification/Item.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/verification/Nym.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/verification/Set.hpp"
)
target_link_libraries(
  opentxs-identity-wot-verification PRIVATE opentxs::messages
)
target_sources(opentxs-identity-wot-verification PRIVATE ${cxx-install-headers})
target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-identity-wot-verification>
)
add_dependencies(opentxs-identity-wot-verification generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/identity/wot/verification
    COMPONENT dev
  )
endif()

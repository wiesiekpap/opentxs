# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(LIBSECP256K1_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Secp256k1.hpp"
      "Secp256k1.cpp"
      "Secp256k1.hpp"
  )

  if(OT_BUNDLED_SECP256K1)
    target_link_libraries(opentxs-common PRIVATE secp256k1-obj)
    target_sources(opentxs PRIVATE $<TARGET_OBJECTS:secp256k1-obj>)
  else()
    target_link_libraries(opentxs-common PRIVATE unofficial::secp256k1)
    target_link_libraries(opentxs PUBLIC unofficial::secp256k1)
  endif()
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()

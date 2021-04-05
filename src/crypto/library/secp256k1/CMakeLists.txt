# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-crypto-library-secp256k1 OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Secp256k1.hpp"
  "Secp256k1.cpp"
  "Secp256k1.hpp"
)
target_link_libraries(
  opentxs-crypto-library-secp256k1 PRIVATE unofficial::secp256k1
)
target_link_libraries(opentxs PUBLIC unofficial::secp256k1)

target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-crypto-library-secp256k1>
)
add_dependencies(opentxs-crypto-library-secp256k1 generated_code)

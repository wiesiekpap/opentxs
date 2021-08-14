# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-crypto-library-sodium OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Sodium.hpp"
  "Sodium.cpp"
  "Sodium.hpp"
)
target_link_libraries(
  opentxs-crypto-library-sodium
  PRIVATE
    opentxs-argon2
    opentxs::messages
    unofficial-sodium::sodium
)
target_link_libraries(opentxs PUBLIC unofficial-sodium::sodium)

target_include_directories(
  opentxs-crypto-library-sodium SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps"
)

if(WIN32 AND OT_STATIC_DEPENDENCIES)
  target_compile_definitions(
    opentxs-crypto-library-sodium PRIVATE SODIUM_STATIC=1 SODIUM_EXPORT=
  )
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-crypto-library-sodium>)
add_dependencies(opentxs-crypto-library-sodium generated_code)

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-lucre OBJECT
  "${CMAKE_CURRENT_BINARY_DIR}/LucreConfig.cpp"
  "Lucre.cpp"
  "Lucre.hpp"
  "Mint.cpp"
  "Mint.hpp"
  "Token.cpp"
  "Token.hpp"
)
target_link_libraries(opentxs-lucre PRIVATE opentxs::messages)
target_include_directories(
  opentxs-lucre SYSTEM
  PRIVATE
    "${opentxs_SOURCE_DIR}/deps/"
    "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
    "${opentxs_SOURCE_DIR}/deps/lucre/include"
    "${OPENSSL_INCLUDE_DIR}"
)

if(OT_OPENSSL_FLAVOR_LIBRESSL)
  target_compile_definitions(opentxs-lucre PRIVATE OT_BUNDLED_LIBRESSL)
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-lucre>)
add_dependencies(opentxs-lucre generated_code)

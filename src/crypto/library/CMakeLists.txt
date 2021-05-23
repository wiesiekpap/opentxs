# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OPENSSL_EXPORT)
  add_subdirectory(openssl)
endif()

if(PACKETCRYPT_EXPORT)
  add_subdirectory(packetcrypt)
endif()

if(LIBSECP256K1_EXPORT)
  add_subdirectory(secp256k1)
endif()

add_subdirectory(sodium)

add_library(
  opentxs-crypto-library OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Factory.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Pbkdf2.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Ripemd160.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Scrypt.hpp"
  "trezor/hmac.c"
  "trezor/hmac.h"
  "trezor/memzero.c"
  "trezor/memzero.h"
  "trezor/options.h"
  "trezor/pbkdf2.c"
  "trezor/pbkdf2.h"
  "trezor/ripemd160.c"
  "trezor/ripemd160.h"
  "trezor/sha2.c"
  "trezor/sha2.h"
  "AsymmetricProvider.cpp"
  "AsymmetricProvider.hpp"
  "AsymmetricProviderNull.hpp"
  "EcdsaProvider.cpp"
  "EcdsaProvider.hpp"
  "HashingProvider.cpp"
  "Pbkdf2.cpp"
  "Pbkdf2.hpp"
  "Ripemd160.cpp"
  "Ripemd160.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/AsymmetricProvider.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/EcdsaProvider.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/EncodingProvider.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/HashingProvider.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/SymmetricProvider.hpp"
)
target_link_libraries(
  opentxs-crypto-library PRIVATE opentxs::messages unofficial-sodium::sodium
)
target_link_libraries(opentxs PUBLIC unofficial-sodium::sodium)

if(WIN32 AND OT_STATIC_DEPENDENCIES)
  target_compile_definitions(
    opentxs-crypto-library PRIVATE SODIUM_STATIC=1 SODIUM_EXPORT=
  )
endif()

if(${CMAKE_CXX_COMPILER_ID}
   MATCHES
   Clang
)
  set_source_files_properties(
    "Ripemd160.cpp" PROPERTIES COMPILE_FLAGS "-Wno-reserved-id-macro"
  )
  set_source_files_properties(
    "Pbkdf2.cpp" PROPERTIES COMPILE_FLAGS "-Wno-reserved-id-macro"
  )
endif()

target_sources(opentxs-crypto-library PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-crypto-library>)
add_dependencies(opentxs-crypto-library generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto/library
    COMPONENT dev
  )
endif()

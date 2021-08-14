# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(base64)
add_subdirectory(irrxml)

if(PACKETCRYPT_EXPORT)
  add_subdirectory(packetcrypt)
endif()

if(CASH_LUCRE_EXPORT)
  add_library(lucre OBJECT "lucre/src/bankimp.cpp")
  target_link_libraries(lucre PRIVATE OpenSSL::Crypto)
  target_include_directories(
    lucre SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/"
  )

  if(OT_OPENSSL_FLAVOR_LIBRESSL)
    target_compile_definitions(lucre PUBLIC OT_BUNDLED_LIBRESSL)
  endif()

  if(NOT MSVC)
    target_compile_options(
      lucre
      PRIVATE
        -Wno-effc++
        -Wno-format
        -Wno-old-style-cast
        -Wno-sign-conversion
        -Wno-unused-variable
        -Wno-zero-as-null-pointer-constant
    )
  endif()

  if(${CMAKE_CXX_COMPILER_ID}
     MATCHES
     Clang
  )
    target_compile_options(
      lucre PRIVATE -Wno-shorten-64-to-32 -Wno-string-conversion
    )
  endif()

  target_sources(opentxs PRIVATE $<TARGET_OBJECTS:lucre>)
endif(CASH_LUCRE_EXPORT)

add_library(
  external-hashes OBJECT "smhasher/src/MurmurHash3.cpp"
                         "smhasher/src/MurmurHash3.h"
)

if(NOT MSVC)
  target_compile_options(
    external-hashes
    PRIVATE
      -Wno-cast-align
      -Wno-effc++
      -Wno-implicit-fallthrough
      -Wno-old-style-cast
      -Wno-shadow
      -Wno-sign-conversion
      -Wno-switch-default
      -Wno-zero-as-null-pointer-constant
  )
endif()

if(${CMAKE_CXX_COMPILER_ID}
   MATCHES
   Clang
)
  target_compile_options(
    external-hashes PRIVATE -Wno-reserved-id-macro
                            -Wno-shadow-field-in-constructor
  )
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:external-hashes>)

add_library(
  bech32 OBJECT
  "bech32/ref/c++/bech32.cpp"
  "bech32/ref/c++/bech32.h"
  "bech32/ref/c++/segwit_addr.cpp"
  "bech32/ref/c++/segwit_addr.h"
)
target_include_directories(
  bech32 SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/bech32/ref/c++"
)

if(NOT MSVC)
  target_compile_options(bech32 PRIVATE -Wno-effc++ -Wno-unused-variable)
endif()

set_target_properties(
  bech32 PROPERTIES C_INCLUDE_WHAT_YOU_USE "" CXX_INCLUDE_WHAT_YOU_USE ""
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:bech32>)

if(OT_BUNDLED_SECP256K1 AND OT_CRYPTO_USING_LIBSECP256K1)
  set(SECP256K1_ENABLE_COVERAGE
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_ENABLE_BRANCH_COVERAGE
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_BUILD_TEST
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_BUILD_BENCH
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_ENABLE_MODULE_ECDH
      ON
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_ECMULT_STATIC_PRECOMPUTATION
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_ENABLE_JNI
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  add_subdirectory(secp256k1)
  set_target_properties(
    secp256k1-obj PROPERTIES C_INCLUDE_WHAT_YOU_USE "" CXX_INCLUDE_WHAT_YOU_USE
                                                       ""
  )
  set_target_properties(
    secp256k1-lib PROPERTIES C_INCLUDE_WHAT_YOU_USE "" CXX_INCLUDE_WHAT_YOU_USE
                                                       ""
  )
endif()

file(
  READ
  "${CMAKE_CURRENT_SOURCE_DIR}/argon2/include/argon2.h"
  ARGON_HEADER
)
string(
  REPLACE
    "__attribute__((visibility(\"default\")))"
    ""
    ARGON_HEADER
    "${ARGON_HEADER}"
)
string(
  REPLACE
    "__attribute__ ((visibility (\"hidden\")))"
    ""
    ARGON_HEADER
    "${ARGON_HEADER}"
)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/argon2/argon2.h" "${ARGON_HEADER}")

add_library(
  opentxs-argon2 OBJECT
  "argon2/src/blake2/blake2-impl.h"
  "argon2/src/blake2/blake2.h"
  "argon2/src/blake2/blake2b.c"
  "argon2/src/argon2.c"
  "argon2/src/core.c"
  "argon2/src/core.h"
  "argon2/src/encoding.c"
  "argon2/src/encoding.h"
  "argon2/src/thread.h"
)

if(ANDROID)
  target_sources(
    opentxs-argon2 PRIVATE "argon2/src/blake2/blamka-round-ref.h"
                           "argon2/src/ref.c"
  )
else()
  target_sources(
    opentxs-argon2 PRIVATE "argon2/src/blake2/blamka-round-opt.h"
                           "argon2/src/opt.c"
  )
endif()

target_include_directories(
  opentxs-argon2 SYSTEM PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/argon2"
)
target_compile_definitions(
  opentxs-argon2
  PUBLIC "A2_VISCTL"
  PRIVATE "ARGON2_NO_THREADS"
)
set_target_properties(
  opentxs-argon2 PROPERTIES C_INCLUDE_WHAT_YOU_USE "" C_STANDARD 90
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-argon2>)

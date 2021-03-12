# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(irrxml)
add_subdirectory(base64)

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

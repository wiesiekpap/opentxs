# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-crypto-library OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Factory.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Pbkdf2.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Ripemd160.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Scrypt.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Sodium.hpp"
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
  "Sodium.cpp"
  "Sodium.hpp"
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
target_include_directories(
  opentxs-crypto-library SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps"
)

if(LIBSECP256K1_EXPORT)
  target_sources(
    opentxs-crypto-library
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Secp256k1.hpp"
      "Secp256k1.cpp"
      "Secp256k1.hpp"
  )
  target_link_libraries(opentxs-crypto-library PRIVATE unofficial::secp256k1)
  target_link_libraries(opentxs PUBLIC unofficial::secp256k1)
endif()

if(OPENSSL_EXPORT)
  target_sources(
    opentxs-crypto-library
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/crypto/library/OpenSSL.hpp"
      "OpenSSL.cpp"
      "OpenSSL.hpp"
      "OpenSSL_BIO.cpp"
      "OpenSSL_BIO.hpp"
  )
  target_link_libraries(
    opentxs-crypto-library
    PRIVATE
      Boost::headers
      OpenSSL::Crypto
      opentxs-api-crypto
      opentxs::messages
  )
  target_link_libraries(opentxs PUBLIC OpenSSL::Crypto)

  if(WIN32 AND OT_STATIC_DEPENDENCIES)
    target_link_libraries(opentxs-crypto-library PRIVATE CRYPT32.LIB)
    target_link_libraries(opentxs PUBLIC CRYPT32.LIB)
  endif()
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

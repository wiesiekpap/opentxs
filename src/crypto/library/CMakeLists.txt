# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    trezor/hmac.c
    trezor/memzero.c
    trezor/pbkdf2.c
    trezor/ripemd160.c
    trezor/sha2.c
    AsymmetricProvider.cpp
    EcdsaProvider.cpp
    HashingProvider.cpp
    Pbkdf2.cpp
    Ripemd160.cpp
    Sodium.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/AsymmetricProvider.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/EcdsaProvider.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/EncodingProvider.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/HashingProvider.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/SymmetricProvider.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Pbkdf2.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Ripemd160.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Scrypt.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Sodium.hpp"
    trezor/hmac.h
    trezor/memzero.h
    trezor/options.h
    trezor/pbkdf2.h
    trezor/ripemd160.h
    trezor/sha2.h
    AsymmetricProvider.hpp
    AsymmetricProviderNull.hpp
    EcdsaProvider.hpp
    Pbkdf2.hpp
    Ripemd160.hpp
    Sodium.hpp
)

add_library(opentxs-crypto-library OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-crypto-library SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps"
)
target_link_libraries(
  opentxs-crypto-library PRIVATE opentxs::messages unofficial-sodium::sodium
)
add_dependencies(opentxs-crypto-library generated_code)

if(LIBSECP256K1_EXPORT)
  add_library(
    opentxs-crypto-library-libsecp256k1 OBJECT
    Secp256k1.cpp Secp256k1.hpp
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Secp256k1.hpp"
  )
  target_link_libraries(
    opentxs-crypto-library-libsecp256k1 PRIVATE opentxs::messages
                                                unofficial::secp256k1
  )
  add_dependencies(opentxs-crypto-library-libsecp256k1 generated_code)
endif()

if(OPENSSL_EXPORT)
  add_library(
    opentxs-crypto-library-openssl OBJECT
    OpenSSL.cpp
    OpenSSL_BIO.cpp
    OpenSSL.hpp
    OpenSSL_BIO.hpp
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/OpenSSL.hpp"
  )
  target_include_directories(
    opentxs-crypto-library-openssl SYSTEM PRIVATE "${OPENSSL_INCLUDE_DIR}"
  )
  target_link_libraries(
    opentxs-crypto-library-openssl PRIVATE Boost::headers opentxs::messages
                                           opentxs-api-crypto
  )
  add_dependencies(opentxs-crypto-library-openssl generated_code)
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto/library
    COMPONENT dev
  )
endif()
